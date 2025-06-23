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

    printf("BME280 Sensor Demo\n");

    bme280_init(&busHandle, &sensorHandle, BME280_SENSOR_ADDR, BME_SDA_IO, BME_SCL_IO, 100000);

    bme280_reset(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    id = bme280_read_id(sensorHandle);
    printf("BME280 ID: 0x%02X\n", id);

    // Configura o sensor
    bme280_config_sensor(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1)
    {
        bme280_read_data(sensorHandle, &sensorData);

        printf("Temperature: %.2f °C\n", sensorData.temperature);
        printf("Pressure: %.2f hPa\n", sensorData.pressure);
        printf("Humidity: %.2f %%\n", sensorData.humidity);
        printf("-------------------\n");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
