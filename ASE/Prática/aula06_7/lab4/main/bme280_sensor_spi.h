#ifndef __BME280_SENSOR_SPI_H__
#define __BME280_SENSOR_SPI_H__

#include "driver/spi_master.h"

// Registradores do BME280 (mesmos do I2C)
#define BME280_REG_ID 0xD0
#define BME280_REG_RESET 0xE0
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_STATUS 0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_PRESS_MSB 0xF7

// Estrutura para armazenar os dados do sensor
typedef struct
{
    float temperature;
    float pressure;
    float humidity;
} bme280_data_t;

// Funções do driver SPI
void bme280_spi_init(spi_device_handle_t *pSensorHandle,
                     int mosiPin, int misoPin, int sclkPin, int csPin,
                     uint32_t clkSpeedHz);

void bme280_spi_free(spi_device_handle_t sensorHandle);

void bme280_spi_reset(spi_device_handle_t sensorHandle);

uint8_t bme280_spi_read_id(spi_device_handle_t sensorHandle);

void bme280_spi_config_sensor(spi_device_handle_t sensorHandle);

void bme280_spi_read_data(spi_device_handle_t sensorHandle, bme280_data_t *data);

#endif // __BME280_SENSOR_SPI_H__
