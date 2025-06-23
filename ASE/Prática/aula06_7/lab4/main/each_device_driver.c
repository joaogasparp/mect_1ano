#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bme280_sensor_spi.h"

#define BME_MOSI_IO 23
#define BME_MISO_IO 19
#define BME_SCLK_IO 18
#define BME_CS_IO 5

void app_main(void)
{
    spi_device_handle_t sensorHandle;
    bme280_data_t sensorData;
    uint8_t id;

    bme280_spi_init(&sensorHandle, BME_MOSI_IO, BME_MISO_IO, BME_SCLK_IO, BME_CS_IO, 1000000);

    while (1)
    {
        bme280_spi_reset(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    /*
    while (1)
    {
        id = bme280_spi_read_id(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    while (1)
    {
        bme280_spi_config_sensor(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    bme280_spi_config_sensor(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1)
    {
        bme280_spi_read_data(sensorHandle, &sensorData);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */
}
