#include <stdio.h>
#include "bme280_sensor_i2c.h"

#define BME_SDA_IO 21
#define BME_SCL_IO 22

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;
    bme280_data_t sensorData;

    // Inicializa o sensor
    bme280_init(&busHandle, &sensorHandle, BME280_SENSOR_ADDR, BME_SDA_IO, BME_SCL_IO, 100000);

    // Reseta o sensor
    bme280_reset(sensorHandle);

    // Lê os dados do sensor
    bme280_read_data(sensorHandle, &sensorData);

    // Imprime os dados no console
    printf("Temperature: %.2f °C\n", sensorData.temperature);
    printf("Pressure: %.2f hPa\n", sensorData.pressure);
    printf("Humidity: %.2f %%\n", sensorData.humidity);

    // Libera os recursos
    bme280_free(busHandle, sensorHandle);
}
