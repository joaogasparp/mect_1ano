#ifndef __BME280_SENSOR_I2C_H__
#define __BME280_SENSOR_I2C_H__

#include "driver/i2c_master.h"

// Endereço padrão do BME280
#define BME280_SENSOR_ADDR 0x76

// Registradores do BME280
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

// Funções do driver
void bme280_init(i2c_master_bus_handle_t *pBusHandle,
                 i2c_master_dev_handle_t *pSensorHandle,
                 uint8_t sensorAddr, int sdaPin, int sclPin, uint32_t clkSpeedHz);

void bme280_free(i2c_master_bus_handle_t busHandle,
                 i2c_master_dev_handle_t sensorHandle);

void bme280_reset(i2c_master_dev_handle_t sensorHandle);

uint8_t bme280_read_id(i2c_master_dev_handle_t sensorHandle);

void bme280_config_sensor(i2c_master_dev_handle_t sensorHandle);

void bme280_read_data(i2c_master_dev_handle_t sensorHandle, bme280_data_t *data);

#endif // __BME280_SENSOR_I2C_H__
