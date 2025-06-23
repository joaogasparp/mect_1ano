#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bme280_sensor_i2c.h"

#define BME_SDA_IO 21
#define BME_SCL_IO 22

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;
    bme280_data_t sensorData;
    uint8_t id;

    bme280_init(&busHandle, &sensorHandle, BME280_SENSOR_ADDR, BME_SDA_IO, BME_SCL_IO, 100000);

    while (1)
    {
        bme280_reset(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    /*
    while (1)
    {
        id = bme280_read_id(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    while (1)
    {
        bme280_config_sensor(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    bme280_config_sensor(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1)
    {
        bme280_read_data(sensorHandle, &sensorData);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */
}
