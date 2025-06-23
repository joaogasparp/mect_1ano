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

    printf("BME280 Sensor Demo (SPI)\n");

    // Inicializa o sensor via SPI
    bme280_spi_init(&sensorHandle, BME_MOSI_IO, BME_MISO_IO, BME_SCLK_IO, BME_CS_IO, 1000000);

    // Reseta o sensor
    bme280_spi_reset(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Lê o ID do sensor
    id = bme280_spi_read_id(sensorHandle);
    printf("BME280 ID: 0x%02X\n", id);

    // Configura o sensor
    bme280_spi_config_sensor(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Loop principal - lê dados a cada 2 segundos
    while (1)
    {
        bme280_spi_read_data(sensorHandle, &sensorData);

        printf("Temperature: %.2f °C\n", sensorData.temperature);
        printf("Pressure: %.2f hPa\n", sensorData.pressure);
        printf("Humidity: %.2f %%\n", sensorData.humidity);
        printf("-------------------\n");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
