#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow_example.h"
#include "esp_http_client.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

static const char *TAG = "espnow_example";

#define ESPNOW_MAXDELAY 512
#define WIFI_CONNECTED_BIT BIT0

#define BUZZER_GPIO     0
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_13_BIT  // 13-bit duty resolution (0-8191)
#define LEDC_FREQUENCY  2500               // Frequency in Hz
#define MAX_VOLUME      100                 // Maximum volume level

static void buzzer_init(void) {
    ESP_LOGI(TAG, "Initializing buzzer on GPIO %d", BUZZER_GPIO);
    
    // Configure timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // Configure channel
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL,
        .duty       = 0,
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = LEDC_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static void buzzer_on(uint8_t volume) {
    // Ensure volume is in valid range
    if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }
    
    // Convert volume percentage to duty cycle (0-8191)
    uint32_t duty = (8191 * volume) / 100;
    ESP_LOGI(TAG, "Turning buzzer on, volume: %d%% (duty: %lu)", volume, duty);
    
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

// Function to turn the buzzer off
static void buzzer_off(void) {
    ESP_LOGI(TAG, "Turning buzzer off");
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

// Task to turn off buzzer after delay
static void buzzer_off_task(void *pvParameters) {
    // Wait for 2 seconds
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    // Turn off the buzzer
    buzzer_off();
    ESP_LOGI(TAG, "Buzzer turned off after delay");
    
    // Delete this task
    vTaskDelete(NULL);
}

static void buzzer_alert_task(void *pvParameters) {
    const int BEEP_COUNT = 6;       // Reduced number of beeps
    const int BEEP_ON_TIME = 200;    // Same as test beep: 200ms on
    const int BEEP_OFF_TIME = 100;   // Same as test beep: 100ms off
    const uint8_t VOLUME = 80;       // Same volume as test beep
    
    ESP_LOGI(TAG, "Starting buzzer alert pattern: %d beeps", BEEP_COUNT);
    
    for (int i = 0; i < BEEP_COUNT; i++) {
        // Turn buzzer on with same volume as test
        buzzer_on(VOLUME);
        ESP_LOGI(TAG, "Beep %d ON", i+1);
        vTaskDelay(BEEP_ON_TIME / portTICK_PERIOD_MS);
        
        // Turn buzzer off
        buzzer_off();
        ESP_LOGI(TAG, "Beep %d OFF", i+1);
        
        // Wait between beeps
        vTaskDelay(BEEP_OFF_TIME / portTICK_PERIOD_MS);
    }
    
    ESP_LOGI(TAG, "Buzzer alert pattern complete");
    vTaskDelete(NULL);
}

static EventGroupHandle_t wifi_event_group;

static QueueHandle_t s_example_espnow_queue;

static uint8_t s_example_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint16_t s_example_espnow_seq[EXAMPLE_ESPNOW_DATA_MAX] = { 0, 0 };

static void example_espnow_deinit(example_espnow_send_param_t *send_param);

typedef struct {
    float bme_temperature;  // in 째C
    float bme_pressure;     // in hPa
    float bme_humidity;     // in %RH
    uint8_t tc74_temperature; // in ºC
    uint32_t timestamp;     // Tick count
} sensor_data_t;

static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    example_espnow_event_t evt;
    example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

// Global variable to store the latest received sensor data
static sensor_data_t latest_sensor_data = {0};
static SemaphoreHandle_t sensor_data_mutex = NULL;
static bool new_data_available = false;

static void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    if (data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    // Check if the received data is a sensor_data_t structure.
    if (len == sizeof(sensor_data_t)) {
        // Create a mutex if it doesn't exist yet
        if (sensor_data_mutex == NULL) {
            sensor_data_mutex = xSemaphoreCreateMutex();
        }
        
        // Take the mutex to safely update the shared data
        if (xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE) {
            // Copy received data to our global variable
            memcpy(&latest_sensor_data, data, sizeof(sensor_data_t));
            new_data_available = true;
            xSemaphoreGive(sensor_data_mutex);
        }
        
        // Log the received data
        sensor_data_t recvData;
        memcpy(&recvData, data, sizeof(sensor_data_t));
        ESP_LOGI(TAG, "Received Sensor Data from " MACSTR ":", MAC2STR(recv_info->src_addr));
        ESP_LOGI(TAG, "  BME Temperature: %.2f °C", recvData.bme_temperature);
        ESP_LOGI(TAG, "  BME Pressure: %.2f hPa", recvData.bme_pressure);
        ESP_LOGI(TAG, "  BME Humidity: %.2f %%RH", recvData.bme_humidity);
        ESP_LOGI(TAG, "  TC74 Temperature: %d °C", recvData.tc74_temperature);
        ESP_LOGI(TAG, "  Timestamp: %lu ticks", recvData.timestamp);

        if (recvData.bme_temperature >= 28.0 || recvData.tc74_temperature >= 28) {
            ESP_LOGI(TAG, "ALERT: High temperature detected! Activating buzzer.");
            xTaskCreate(buzzer_alert_task, "buzzer_alert_task", 2048, NULL, 5, NULL);
        }
    }

    example_espnow_event_t evt;
    example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t * mac_addr = recv_info->src_addr;
    uint8_t * des_addr = recv_info->des_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    if (IS_BROADCAST_ADDR(des_addr)) {
        ESP_LOGD(TAG, "Receive broadcast ESPNOW data");
    } else {
        ESP_LOGD(TAG, "Receive unicast ESPNOW data");
    }

    evt.id = EXAMPLE_ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

int example_espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, uint32_t *magic)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(example_espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    *state = buf->state;
    *seq = buf->seq_num;
    *magic = buf->magic;
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) {
        return buf->type;
    }

    return -1;
}

void example_espnow_data_prepare(example_espnow_send_param_t *send_param)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)send_param->buffer;

    assert(send_param->len >= sizeof(example_espnow_data_t));

    buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? EXAMPLE_ESPNOW_DATA_BROADCAST : EXAMPLE_ESPNOW_DATA_UNICAST;
    buf->state = send_param->state;
    buf->seq_num = s_example_espnow_seq[buf->type]++;
    buf->crc = 0;
    buf->magic = send_param->magic;
    esp_fill_random(buf->payload, send_param->len - sizeof(example_espnow_data_t));
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

static void example_espnow_task(void *pvParameter)
{
    example_espnow_event_t evt;
    uint8_t recv_state = 0;
    uint16_t recv_seq = 0;
    uint32_t recv_magic = 0;
    bool is_broadcast = false;
    int ret;

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Start sending broadcast data");

    example_espnow_send_param_t *send_param = (example_espnow_send_param_t *)pvParameter;
    if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        example_espnow_deinit(send_param);
        vTaskDelete(NULL);
    }

    while (xQueueReceive(s_example_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case EXAMPLE_ESPNOW_SEND_CB:
            {
                example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
                is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

                ESP_LOGD(TAG, "Send data to "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);

                if (is_broadcast && (send_param->broadcast == false)) {
                    break;
                }

                if (!is_broadcast) {
                    send_param->count--;
                    if (send_param->count == 0) {
                        ESP_LOGI(TAG, "Send done");
                        example_espnow_deinit(send_param);
                        vTaskDelete(NULL);
                    }
                }

                if (send_param->delay > 0) {
                    vTaskDelay(send_param->delay/portTICK_PERIOD_MS);
                }

                ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(send_cb->mac_addr));

                memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);
                example_espnow_data_prepare(send_param);

                if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                    ESP_LOGE(TAG, "Send error");
                    example_espnow_deinit(send_param);
                    vTaskDelete(NULL);
                }
                break;
            }
            case EXAMPLE_ESPNOW_RECV_CB:
            {
                example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

                ret = example_espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
                free(recv_cb->data);
                if (ret == EXAMPLE_ESPNOW_DATA_BROADCAST) {
                    ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    if (esp_now_is_peer_exist(recv_cb->mac_addr) == false) {
                        esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
                        if (peer == NULL) {
                            ESP_LOGE(TAG, "Malloc peer information fail");
                            example_espnow_deinit(send_param);
                            vTaskDelete(NULL);
                        }
                        memset(peer, 0, sizeof(esp_now_peer_info_t));
                        peer->channel = CONFIG_ESPNOW_CHANNEL;
                        peer->ifidx = ESPNOW_WIFI_IF;
                        peer->encrypt = true;
                        memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
                        memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
                        free(peer);
                    }

                    if (send_param->state == 0) {
                        send_param->state = 1;
                    }

                    if (recv_state == 1) {
                        if (send_param->unicast == false && send_param->magic >= recv_magic) {
                    	    ESP_LOGI(TAG, "Start sending unicast data");
                    	    ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(recv_cb->mac_addr));

                            memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                            example_espnow_data_prepare(send_param);
                            if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                                ESP_LOGE(TAG, "Send error");
                                example_espnow_deinit(send_param);
                                vTaskDelete(NULL);
                            }
                            else {
                                send_param->broadcast = false;
                                send_param->unicast = true;
                            }
                        }
                    }
                }
                else if (ret == EXAMPLE_ESPNOW_DATA_UNICAST) {
                    ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    send_param->broadcast = false;
                }
                else {
                    ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
                }
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}

static esp_err_t example_espnow_init(void)
{
    example_espnow_send_param_t *send_param;

    s_example_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_t));
    if (s_example_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(example_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );
#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
    ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
#endif
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    send_param = malloc(sizeof(example_espnow_send_param_t));
    if (send_param == NULL) {
        ESP_LOGE(TAG, "Malloc send parameter fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(send_param, 0, sizeof(example_espnow_send_param_t));
    send_param->unicast = false;
    send_param->broadcast = true;
    send_param->state = 0;
    send_param->magic = esp_random();
    send_param->count = CONFIG_ESPNOW_SEND_COUNT;
    send_param->delay = CONFIG_ESPNOW_SEND_DELAY;
    send_param->len = CONFIG_ESPNOW_SEND_LEN;
    send_param->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
    if (send_param->buffer == NULL) {
        ESP_LOGE(TAG, "Malloc send buffer fail");
        free(send_param);
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memcpy(send_param->dest_mac, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    example_espnow_data_prepare(send_param);

    xTaskCreate(example_espnow_task, "example_espnow_task", 2048, send_param, 4, NULL);

    return ESP_OK;
}

static void example_espnow_deinit(example_espnow_send_param_t *send_param)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(s_example_espnow_queue);
    esp_now_deinit();
}

static void sensor_post_task(void *pvParameters)
{
    sensor_data_t sensorData;
    char post_data[256];
    int retry_count = 0;
    bool data_sent = false;
    
    // Wait a bit longer for WiFi to stabilize before first attempt
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    while (1) {
        // Check if we have new sensor data to send
        if (sensor_data_mutex != NULL && xSemaphoreTake(sensor_data_mutex, portMAX_DELAY) == pdTRUE) {
            if (new_data_available) {
                // Copy the latest sensor data
                memcpy(&sensorData, &latest_sensor_data, sizeof(sensor_data_t));
                new_data_available = false;
                data_sent = true;
                xSemaphoreGive(sensor_data_mutex);
                
                ESP_LOGI(TAG, "Sending real sensor data to Flask:");
                ESP_LOGI(TAG, "  BME Temperature: %.2f °C", sensorData.bme_temperature);
                ESP_LOGI(TAG, "  BME Pressure: %.2f hPa", sensorData.bme_pressure);
                ESP_LOGI(TAG, "  BME Humidity: %.2f %%RH", sensorData.bme_humidity);
                ESP_LOGI(TAG, "  TC74 Temperature: %d °C", sensorData.tc74_temperature);
                
                snprintf(post_data, sizeof(post_data),
                        "{\"bme_temperature\":%.2f, \"bme_pressure\":%.2f, \"bme_humidity\":%.2f, \"tc74_temperature\":%d, \"timestamp\":%lu}",
                        sensorData.bme_temperature, sensorData.bme_pressure, sensorData.bme_humidity,
                        sensorData.tc74_temperature, sensorData.timestamp);

                esp_http_client_config_t config = {
                    .url = "http://192.168.223.169:3000/update",
                    .method = HTTP_METHOD_POST,
                    .timeout_ms = 15000,  // Increased timeout
                    .skip_cert_common_name_check = true,
                    .keep_alive_enable = false, // Add this to close connection after each request
                };
                
                ESP_LOGI(TAG, "Sending POST to Flask server at %s", config.url);
                
                // Connection diagnostic before attempt
                uint32_t start_time = xTaskGetTickCount();
                ESP_LOGI(TAG, "Starting HTTP connection at tick: %lu", start_time);
                
                esp_http_client_handle_t client = esp_http_client_init(&config);
                esp_http_client_set_header(client, "Content-Type", "application/json");
                esp_http_client_set_post_field(client, post_data, strlen(post_data));
                
                esp_err_t err = esp_http_client_perform(client);
                
                uint32_t end_time = xTaskGetTickCount();
                ESP_LOGI(TAG, "HTTP attempt finished at tick: %lu (took %lu ms)", 
                         end_time, (end_time - start_time) * portTICK_PERIOD_MS);
                
                if (err == ESP_OK) {
                    int status_code = esp_http_client_get_status_code(client);
                    ESP_LOGI(TAG, "HTTP POST Status = %d", status_code);
                    ESP_LOGI(TAG, "Real sensor data sent to Flask successfully");
                    retry_count = 0;
                } else {
                    ESP_LOGE(TAG, "HTTP POST failed: %s (err = %d)", esp_err_to_name(err), err);
                    retry_count++;
                    
                    if (retry_count > 3) { // Reduced from 5 to 3
                        ESP_LOGE(TAG, "Too many failures, reconnecting WiFi...");
                        esp_wifi_disconnect();
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                        esp_wifi_connect();
                        retry_count = 0;
                        vTaskDelay(3000 / portTICK_PERIOD_MS); // Wait for reconnection
                    }
                }
                
                esp_http_client_cleanup(client);
            } else {
                xSemaphoreGive(sensor_data_mutex);
                if (!data_sent) {
                    ESP_LOGI(TAG, "No new sensor data available yet, waiting...");
                }
            }
        }
        
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Check for new data every 2 seconds
    }
}

static void network_test_task(void *pvParameters) {
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                          WIFI_CONNECTED_BIT,
                                          pdFALSE,
                                          pdTRUE,
                                          portMAX_DELAY);
                                          
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Network test task running - WiFi connected");
        
        // Create a simple GET request to test network
        esp_http_client_config_t config = {
            .url = "http://192.168.223.169:3000/",
            .method = HTTP_METHOD_GET,
            .timeout_ms = 5000,
        };
        
        // Try connection test a few times
        for (int i = 0; i < 3; i++) {
            ESP_LOGI(TAG, "Testing network connection attempt %d", i+1);
            esp_http_client_handle_t client = esp_http_client_init(&config);
            
            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK) {
                int status_code = esp_http_client_get_status_code(client);
                ESP_LOGI(TAG, "NETWORK TEST SUCCESS - HTTP Status = %d", status_code);
                esp_http_client_cleanup(client);
                vTaskDelete(NULL);  // Test successful, delete task
            } else {
                ESP_LOGE(TAG, "Network test failed: %s (err = %d)", esp_err_to_name(err), err);
            }
            
            esp_http_client_cleanup(client);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        
        ESP_LOGE(TAG, "Network test failed after multiple attempts - check server and network");
    }
    vTaskDelete(NULL);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "======================================");
        ESP_LOGI(TAG, "Wi-Fi CONNECTED! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Starting HTTP sensor task NOW!");
        ESP_LOGI(TAG, "======================================");
        
        // Create sensor post task with higher priority (6) than ESPNOW (4)
        xTaskCreate(sensor_post_task, "sensor_post_task", 4096, NULL, 6, NULL);
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi started, connecting to AP...");
        esp_wifi_connect();
    }
}

/* WiFi should start before using ESPNOW */
static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi_event_group = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Kirin",
            .password = "minhanetsai",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // Register event handlers BEFORE starting WiFi
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                       ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    // Register only ONCE for IP events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                       IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    
    // Start WiFi - the event handler will call esp_wifi_connect() 
    // when it receives WIFI_EVENT_STA_START
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // REMOVE THIS LINE - let the event handler handle connection
    // ESP_ERROR_CHECK(esp_wifi_connect());
    
#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESPNOW_WIFI_IF,
              WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));
#endif
}


static void test_http_task(void *pvParameters)
{
    // Wait for Wi-Fi connection
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Running direct HTTP test to Flask server");
    
    sensor_data_t testData = {
        .bme_temperature = 22.5,
        .bme_pressure = 1013.25,
        .bme_humidity = 45.0,
        .tc74_temperature = 23,
        .timestamp = xTaskGetTickCount()
    };
    
    char post_data[256];
    snprintf(post_data, sizeof(post_data),
            "{\"bme_temperature\":%.2f, \"bme_pressure\":%.2f, \"bme_humidity\":%.2f, \"tc74_temperature\":%d, \"timestamp\":%lu}",
            testData.bme_temperature, testData.bme_pressure, testData.bme_humidity,
            testData.tc74_temperature, testData.timestamp);
    
    esp_http_client_config_t config = {
        .url = "http://192.168.223.169:3000/update",
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .skip_cert_common_name_check = true,
    };
    
    ESP_LOGI(TAG, "Sending test POST to Flask server at %s", config.url);
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "TEST HTTP POST Status = %d", status_code);
        ESP_LOGI(TAG, "Test data sent to Flask successfully");
    } else {
        ESP_LOGE(TAG, "TEST HTTP POST failed: %s (err = %d)", esp_err_to_name(err), err);
    }
    
    esp_http_client_cleanup(client);
    
    // Loop with periodic attempts
    while(1) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Sending another test POST");
        
        // Create a new client for each request
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "TEST HTTP POST Status = %d", status_code);
            ESP_LOGI(TAG, "Test data sent to Flask successfully");
        } else {
            ESP_LOGE(TAG, "TEST HTTP POST failed: %s (err = %d)", esp_err_to_name(err), err);
        }
        
        esp_http_client_cleanup(client);
        ESP_LOGI(TAG, "Waiting for 10 seconds before next attempt...");
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize buzzer using LEDC
    buzzer_init();

    buzzer_on(80);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    buzzer_off();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    buzzer_on(80);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    buzzer_off();

    
    example_wifi_init();
    
    xTaskCreate(network_test_task, "network_test_task", 4096, NULL, 5, NULL);
    xTaskCreate(test_http_task, "test_http_task", 4096, NULL, 5, NULL);


    vTaskDelay(3000 / portTICK_PERIOD_MS);

    example_espnow_init();
}