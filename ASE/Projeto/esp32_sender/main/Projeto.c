#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "esp_system.h"

#include "bme280_sensor_i2c.h"
#include "temp_sensor_tc74.h"
#include "espnow_example.h"
#include "esp_rom_sys.h"

#define BME280_SDA_IO        2
#define BME280_SCL_IO        3
#define BME280_CLK_HZ        100000
#define BME280_SENSOR_ADDR   BME280_DEFAULT_ADDR

#define TC74_SDA_IO          2
#define TC74_SCL_IO          3
#define TC74_SENSOR_ADDR     TC74_A1_SENSOR_ADDR
#define TC74_CLK_FREQ_HZ     TC74_SCL_DFLT_FREQ_HZ

static const char *TAG = "espnow_sensor";

// SPIFFS configuration
#define SPIFFS_PARTITION_LABEL   "storage"
#define SPIFFS_MAX_FILES         5
#define LOG_FILE_PATH            "/spiffs/sensor_log.csv"

// Flag to track WiFi/ESPNOW connection status
static bool espnow_send_success = true;
static uint32_t last_successful_send = 0;
static const uint32_t CONNECTION_TIMEOUT_MS = 10000; // 10 seconds timeout

// Structure for sensor data to be sent via ESPNOW.
typedef struct {
    float bme_temperature;  // in °C
    float bme_pressure;     // in hPa
    float bme_humidity;     // in %RH
    uint8_t tc74_temperature; // in °C
    uint32_t timestamp;     // Tick count
} sensor_data_t;

// Function to initialize SPIFFS
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = SPIFFS_PARTITION_LABEL,
        .max_files = SPIFFS_MAX_FILES,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "SPIFFS Partition: total: %d, used: %d", total, used);
    
    // Create log file if it doesn't exist
    FILE *f = fopen(LOG_FILE_PATH, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    
    // Write CSV header if file is empty
    if (ftell(f) == 0) {
        fprintf(f, "timestamp,bme_temperature,bme_pressure,bme_humidity,tc74_temperature\n");
    }
    
    fclose(f);
    return ESP_OK;
}

// Function to save sensor data to SPIFFS
static esp_err_t save_sensor_data_to_spiffs(sensor_data_t *data)
{
    FILE *f = fopen(LOG_FILE_PATH, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    
    // Write data in CSV format
    fprintf(f, "%lu,%.2f,%.2f,%.2f,%d\n", 
            data->timestamp, 
            data->bme_temperature, 
            data->bme_pressure, 
            data->bme_humidity, 
            data->tc74_temperature);
    
    fclose(f);
    ESP_LOGI(TAG, "Sensor data saved to SPIFFS");
    return ESP_OK;
}

// ESPNOW send callback function
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "ESPNOW data sent successfully");
        espnow_send_success = true;
        last_successful_send = xTaskGetTickCount() * portTICK_PERIOD_MS;
    } else {
        ESP_LOGE(TAG, "ESPNOW data send failed");
        espnow_send_success = false;
    }
}

// Sensor reading and ESPNOW sending task.
static void sensor_send_task(void *pvParameters)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;
    sensor_data_t data;
    uint8_t chip_id;
    uint32_t raw_temp, raw_pressure;
    uint16_t raw_humidity;
    bme280_calib_t calib;
    esp_err_t ret;
    
    ESP_ERROR_CHECK(bme280_init(&busHandle, &sensorHandle, BME280_SENSOR_ADDR, BME280_SDA_IO, BME280_SCL_IO, BME280_CLK_HZ));
    ESP_ERROR_CHECK(bme280_configure(sensorHandle));
    ESP_ERROR_CHECK(bme280_read_calibration(sensorHandle, &calib));
    
    // Initialize connection status
    last_successful_send = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    while (1) {
        // Get current tick count.
        data.timestamp = xTaskGetTickCount();
        
        // Read BME280 sensor.
        ret = bme280_read_chip_id(sensorHandle, &chip_id);
        ret |= bme280_read_raw_temp(sensorHandle, &raw_temp);
        ret |= bme280_read_raw_pressure(sensorHandle, &raw_pressure);
        ret |= bme280_read_raw_humidity(sensorHandle, &raw_humidity);
        if (ret == ESP_OK) {
            int32_t temp_c100 = bme280_compensate_temperature((int32_t)raw_temp, &calib);
            uint32_t pressure_pa = bme280_compensate_pressure((int32_t)raw_pressure, &calib);
            uint32_t hum_q22 = bme280_compensate_humidity((int32_t)raw_humidity, &calib);
            
            data.bme_temperature = temp_c100 / 100.0f;
            data.bme_pressure = pressure_pa / 25600.0f;
            data.bme_humidity = hum_q22 / 1024.0f;
        } else {
            ESP_LOGE(TAG, "BME280 read error");
        }
        
        tc74_read_temp_after_cfg(sensorHandle, &data.tc74_temperature);

        ESP_LOGI(TAG, "Sensor Data: BME Temperature: %.2f °C, Pressure: %.2f hPa, Humidity: %.2f %%RH, TC74 Temperature: %d °C, Timestamp: %u ticks",
                 data.bme_temperature, data.bme_pressure, data.bme_humidity, data.tc74_temperature, (unsigned)data.timestamp);
        
        // Check if we need to save to SPIFFS due to connection issues
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        bool connection_lost = (current_time - last_successful_send) > CONNECTION_TIMEOUT_MS;
        
        if (connection_lost) {
            ESP_LOGW(TAG, "Connection appears to be lost. Saving data to SPIFFS");
            save_sensor_data_to_spiffs(&data);
        }
        
        // Always try to send data via ESPNOW
        uint8_t dest_mac[ESP_NOW_ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        ret = esp_now_send(dest_mac, (uint8_t *)&data, sizeof(sensor_data_t));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_now_send failed (%d)", ret);
            // Save to SPIFFS on immediate failure
            save_sensor_data_to_spiffs(&data);
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Function to read and send stored data from SPIFFS
static void send_stored_data_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting task to send stored data from SPIFFS");
    
    // Wait a bit to ensure WiFi is stable
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    FILE *f = fopen(LOG_FILE_PATH, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open log file for reading");
        vTaskDelete(NULL);
        return;
    }
    
    char line[256];
    sensor_data_t data;
    int lines_sent = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), f) == NULL) {
        ESP_LOGE(TAG, "Failed to read header or file is empty");
        fclose(f);
        vTaskDelete(NULL);
        return;
    }
    
    // Read and send each line
    while (fgets(line, sizeof(line), f) != NULL) {
        // Parse CSV line
        if (sscanf(line, "%lu,%f,%f,%f,%hhu", 
                  &data.timestamp, 
                  &data.bme_temperature, 
                  &data.bme_pressure, 
                  &data.bme_humidity, 
                  &data.tc74_temperature) == 5) {
            
            ESP_LOGI(TAG, "Sending stored data: BME Temp: %.2f, Pressure: %.2f, Humidity: %.2f, TC74 Temp: %d",
                    data.bme_temperature, data.bme_pressure, data.bme_humidity, data.tc74_temperature);
            
            // Send via ESPNOW
            uint8_t dest_mac[ESP_NOW_ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
            esp_err_t ret = esp_now_send(dest_mac, (uint8_t *)&data, sizeof(sensor_data_t));
            if (ret == ESP_OK) {
                lines_sent++;
                // Small delay between sends to avoid flooding
                vTaskDelay(100 / portTICK_PERIOD_MS);
            } else {
                ESP_LOGE(TAG, "Failed to send stored data");
                break; // If we can't send, stop trying
            }
        }
    }
    
    fclose(f);
    ESP_LOGI(TAG, "Sent %d stored data points from SPIFFS", lines_sent);
    
    if (lines_sent > 0) {
        // Create a new empty log file (with header) if we successfully sent data
        f = fopen(LOG_FILE_PATH, "w");
        if (f != NULL) {
            fprintf(f, "timestamp,bme_temperature,bme_pressure,bme_humidity,tc74_temperature\n");
            fclose(f);
            ESP_LOGI(TAG, "Log file reset after sending stored data");
        }
    }
    
    vTaskDelete(NULL);
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}

void app_main(void)
{
    // Initialize NVS.
    ESP_ERROR_CHECK(nvs_flash_init());
    
    // Initialize SPIFFS
    ESP_ERROR_CHECK(init_spiffs());
    ESP_LOGI(TAG, "SPIFFS initialized successfully");
    
    // Initialize Wi-Fi.
    wifi_init();
    
    // Initialize ESPNOW.
    ESP_ERROR_CHECK(esp_now_init());
    
    // Register send callback to track connection status
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    
    // Add broadcast peer explicitly.
    uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {0};
    memcpy(peerInfo.peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    peerInfo.channel = CONFIG_ESPNOW_CHANNEL;
    peerInfo.ifidx = ESPNOW_WIFI_IF;
    peerInfo.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));

    // Create the sensor reading and sending task.
    xTaskCreate(sensor_send_task, "sensor_send_task", 4096, NULL, 5, NULL);
    
    // Create a task to check and send stored data from SPIFFS
    xTaskCreate(send_stored_data_task, "send_stored_data", 4096, NULL, 4, NULL);
    
    ESP_LOGI(TAG, "ESP32 sensor node initialized with SPIFFS storage");
}