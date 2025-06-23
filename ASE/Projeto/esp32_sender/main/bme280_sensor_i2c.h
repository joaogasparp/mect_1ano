#ifndef BME280_SENSOR_I2C_H
#define BME280_SENSOR_I2C_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdint.h>

#define BME280_DEFAULT_ADDR  0x76

#define BME280_REG_CHIP_ID   0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CALIB00   0x88
#define BME280_REG_CALIB26   0xE1

#define BME280_REG_PRESS_MSB 0xF7
#define BME280_REG_TEMP_MSB  0xFA
#define BME280_REG_HUM_MSB   0xFD

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_t;

esp_err_t bme280_init(i2c_master_bus_handle_t *pBusHandle,
                      i2c_master_dev_handle_t *pSensorHandle,
                      uint8_t sensorAddr,
                      int sdaPin, int sclPin, uint32_t clkSpeedHz);
esp_err_t bme280_free(i2c_master_bus_handle_t busHandle,
                      i2c_master_dev_handle_t sensorHandle);
esp_err_t bme280_read_chip_id(i2c_master_dev_handle_t sensorHandle,
                              uint8_t *chip_id);
esp_err_t bme280_read_raw_temp(i2c_master_dev_handle_t sensorHandle,
                               uint32_t *raw_temp);
esp_err_t bme280_read_raw_pressure(i2c_master_dev_handle_t sensorHandle,
                                   uint32_t *raw_pressure);
esp_err_t bme280_read_raw_humidity(i2c_master_dev_handle_t sensorHandle,
                                   uint16_t *raw_humidity);
esp_err_t bme280_configure(i2c_master_dev_handle_t sensorHandle);
esp_err_t bme280_read_calibration(i2c_master_dev_handle_t sensorHandle,
                                  bme280_calib_t *calib);
int32_t bme280_compensate_temperature(int32_t adc_T, bme280_calib_t *calib);
uint32_t bme280_compensate_pressure(int32_t adc_P, bme280_calib_t *calib);
uint32_t bme280_compensate_humidity(int32_t adc_H, bme280_calib_t *calib);

#endif // BME280_SENSOR_I2C_H