#include <stdio.h>
#include "bme280_sensor_i2c.h"

esp_err_t bme280_init(i2c_master_bus_handle_t *pBusHandle,
                      i2c_master_dev_handle_t *pSensorHandle,
                      uint8_t sensorAddr,
                      int sdaPin, int sclPin, uint32_t clkSpeedHz) {
    i2c_master_bus_config_t i2cMasterCfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = sclPin,
        .sda_io_num = sdaPin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2cMasterCfg, pBusHandle));
    i2c_device_config_t i2cDevCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = sensorAddr,
        .scl_speed_hz = clkSpeedHz,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*pBusHandle, &i2cDevCfg, pSensorHandle));
    return ESP_OK;
}

esp_err_t bme280_free(i2c_master_bus_handle_t busHandle,
                      i2c_master_dev_handle_t sensorHandle) {
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(sensorHandle));
    ESP_ERROR_CHECK(i2c_del_master_bus(busHandle));
    return ESP_OK;
}

esp_err_t bme280_read_chip_id(i2c_master_dev_handle_t sensorHandle,
                              uint8_t *chip_id) {
    uint8_t reg = BME280_REG_CHIP_ID;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), chip_id, sizeof(*chip_id), -1));
    return ESP_OK;
}

esp_err_t bme280_read_raw_temp(i2c_master_dev_handle_t sensorHandle,
                               uint32_t *raw_temp) {
    uint8_t reg = BME280_REG_TEMP_MSB, buf[3];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), buf, sizeof(buf), -1));
    *raw_temp = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4);
    return ESP_OK;
}

esp_err_t bme280_read_raw_pressure(i2c_master_dev_handle_t sensorHandle,
                                   uint32_t *raw_pressure) {
    uint8_t reg = BME280_REG_PRESS_MSB, buf[3];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), buf, sizeof(buf), -1));
    *raw_pressure = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4);
    return ESP_OK;
}

esp_err_t bme280_read_raw_humidity(i2c_master_dev_handle_t sensorHandle,
                                   uint16_t *raw_humidity) {
    uint8_t reg = BME280_REG_HUM_MSB, buf[2];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), buf, sizeof(buf), -1));
    *raw_humidity = ((uint16_t)buf[0] << 8) | buf[1];
    return ESP_OK;
}

esp_err_t bme280_configure(i2c_master_dev_handle_t sensorHandle) {
    uint8_t data1[2] = {0xF2, 0x01};
    uint8_t data2[2] = {0xF5, 0xA0};
    uint8_t data3[2] = {0xF4, 0x27};
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, data1, sizeof(data1), -1));
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, data2, sizeof(data2), -1));
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, data3, sizeof(data3), -1));
    return ESP_OK;
}

esp_err_t bme280_read_calibration(i2c_master_dev_handle_t sensorHandle,
                                  bme280_calib_t *calib) {
    uint8_t buf[26], reg = BME280_REG_CALIB00;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, 1, buf, 26, -1));
    calib->dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    calib->dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
    calib->dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);
    calib->dig_P1 = (uint16_t)(buf[7] << 8 | buf[6]);
    calib->dig_P2 = (int16_t)(buf[9] << 8 | buf[8]);
    calib->dig_P3 = (int16_t)(buf[11] << 8 | buf[10]);
    calib->dig_P4 = (int16_t)(buf[13] << 8 | buf[12]);
    calib->dig_P5 = (int16_t)(buf[15] << 8 | buf[14]);
    calib->dig_P6 = (int16_t)(buf[17] << 8 | buf[16]);
    calib->dig_P7 = (int16_t)(buf[19] << 8 | buf[18]);
    calib->dig_P8 = (int16_t)(buf[21] << 8 | buf[20]);
    calib->dig_P9 = (int16_t)(buf[23] << 8 | buf[22]);
    calib->dig_H1 = buf[25];
    uint8_t buf2[7];
    reg = BME280_REG_CALIB26;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, 1, buf2, 7, -1));
    calib->dig_H2 = (int16_t)(buf2[1] << 8 | buf2[0]);
    calib->dig_H3 = buf2[2];
    calib->dig_H4 = (int16_t)((buf2[3] << 4) | (buf2[4] & 0x0F));
    calib->dig_H5 = (int16_t)((buf2[5] << 4) | (buf2[4] >> 4));
    calib->dig_H6 = (int8_t)buf2[6];
    return ESP_OK;
}

static int32_t t_fine;

int32_t bme280_compensate_temperature(int32_t adc_T, bme280_calib_t *calib) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib->dig_T1 << 1))) * ((int32_t)calib->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib->dig_T1)) * ((adc_T >> 4) - ((int32_t)calib->dig_T1))) >> 12) * ((int32_t)calib->dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t bme280_compensate_pressure(int32_t adc_P, bme280_calib_t *calib) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib->dig_P6;
    var2 += ((var1 * (int64_t)calib->dig_P5) << 17);
    var2 += (((int64_t)calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib->dig_P3) >> 8) + ((var1 * (int64_t)calib->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)calib->dig_P1)) >> 33;
    if(var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib->dig_P7) << 4);
    return (uint32_t)p;
}

uint32_t bme280_compensate_humidity(int32_t adc_H, bme280_calib_t *calib) {
    int32_t v_x1;
    v_x1 = t_fine - 76800;
    v_x1 = (((((adc_H << 14) - (((int32_t)calib->dig_H4) << 20) - (((int32_t)calib->dig_H5) * v_x1)) + 16384) >> 15) *
           (((((((v_x1 * (int32_t)calib->dig_H6) >> 10) * (((v_x1 * (int32_t)calib->dig_H3) >> 11) + 32768)) >> 10) +
           2097152) * (int32_t)calib->dig_H2 + 8192) >> 14));
    v_x1 = v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int32_t)calib->dig_H1) >> 4);
    if(v_x1 < 0) v_x1 = 0;
    if(v_x1 > 419430400) v_x1 = 419430400;
    return (uint32_t)(v_x1 >> 12);
}