#include <stdio.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "temp_sensor_tc74.h"
#include "bme280_sensor_i2c.h"

#define TC74_SDA_IO 3
#define TC74_SCL_IO 2
#define BME_SDA_IO 21
#define BME_SCL_IO 22
#define LM335_ADC_CHANNEL ADC_CHANNEL_2
#define LM335_ADC_ATTEN ADC_ATTEN_DB_12

// Temperaturas atuais de cada sensor
static float tc74_temp = 0.0;
static float bme280_temp = 0.0;
static float lm335_temp = 0.0;

// Handles
static SemaphoreHandle_t dataMutex;
static i2c_master_dev_handle_t tc74_handle;
static i2c_master_dev_handle_t bme280_handle;
static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_handle = NULL;
static bool do_calibration = false;

// Inicializar calibração ADC
static void adc_calibration_init(void)
{
    esp_err_t ret = ESP_FAIL;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = LM335_ADC_CHANNEL,
        .atten = LM335_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
#endif
    if (ret == ESP_OK)
    {
        do_calibration = true;
    }
}

// Task TC74
void tc74_task(void *pvParameters)
{
    uint8_t temp_raw;
    while (1)
    {
        tc74_read_temp_after_temp(tc74_handle, &temp_raw);
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            tc74_temp = (float)temp_raw;
            xSemaphoreGive(dataMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task BME280
void bme280_task(void *pvParameters)
{
    bme280_data_t sensor_data;
    while (1)
    {
        bme280_read_data(bme280_handle, &sensor_data);
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            bme280_temp = sensor_data.temperature;
            xSemaphoreGive(dataMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task LM335
void lm335_task(void *pvParameters)
{
    int adc_raw, voltage_mv;
    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, LM335_ADC_CHANNEL, &adc_raw));
        if (do_calibration)
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage_mv));
            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
            {
                float temp_kelvin = voltage_mv / 10.0;
                lm335_temp = temp_kelvin - 273.15;
                xSemaphoreGive(dataMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task para verificar tecla e mostrar média
void keypress_task(void *pvParameters)
{
    while (1)
    {
        int c = fgetc(stdin);
        if (c != EOF)
        {
            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
            {
                float average = (tc74_temp + bme280_temp + lm335_temp) / 3.0;
                printf("Average Temperature: %.1f °C\n", average);
                xSemaphoreGive(dataMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t tc74_bus, bme280_bus;

    printf("Multi-Sensor Temperature System\n");
    printf("Press any key to see average temperature\n\n");

    // Criar mutex
    dataMutex = xSemaphoreCreateMutex();

    // Inicializar sensores
    tc74_init(&tc74_bus, &tc74_handle, TC74_A5_SENSOR_ADDR, TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);
    tc74_wakeup(tc74_handle);
    vTaskDelay(pdMS_TO_TICKS(250));
    uint8_t temp;
    tc74_read_temp_after_cfg(tc74_handle, &temp);

    bme280_init(&bme280_bus, &bme280_handle, BME280_SENSOR_ADDR, BME_SDA_IO, BME_SCL_IO, 100000);
    bme280_reset(bme280_handle);
    vTaskDelay(pdMS_TO_TICKS(100));
    bme280_config_sensor(bme280_handle);

    adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    adc_oneshot_chan_cfg_t config = {.atten = LM335_ADC_ATTEN, .bitwidth = ADC_BITWIDTH_DEFAULT};
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LM335_ADC_CHANNEL, &config));
    adc_calibration_init();

    // Criar tasks
    xTaskCreate(tc74_task, "TC74_Task", 2048, NULL, 5, NULL);
    xTaskCreate(bme280_task, "BME280_Task", 2048, NULL, 5, NULL);
    xTaskCreate(lm335_task, "LM335_Task", 2048, NULL, 5, NULL);
    xTaskCreate(keypress_task, "Keypress_Task", 2048, NULL, 3, NULL);

    printf("System started!\n");
}
