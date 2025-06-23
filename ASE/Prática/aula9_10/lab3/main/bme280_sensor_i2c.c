#include "bme280_sensor_i2c.h"
#include <math.h>

void bme280_init(i2c_master_bus_handle_t *pBusHandle,
                 i2c_master_dev_handle_t *pSensorHandle,
                 uint8_t sensorAddr, int sdaPin, int sclPin, uint32_t clkSpeedHz)
{
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
}

void bme280_free(i2c_master_bus_handle_t busHandle, i2c_master_dev_handle_t sensorHandle)
{
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(sensorHandle));
    ESP_ERROR_CHECK(i2c_del_master_bus(busHandle));
}

void bme280_reset(i2c_master_dev_handle_t sensorHandle)
{
    uint8_t buffer[2] = {BME280_REG_RESET, 0xB6};
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1));
}

uint8_t bme280_read_id(i2c_master_dev_handle_t sensorHandle)
{
    uint8_t reg = BME280_REG_ID;
    uint8_t id;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), &id, sizeof(id), -1));
    return id;
}

void bme280_config_sensor(i2c_master_dev_handle_t sensorHandle)
{
    // Configurar humidade (oversampling x1)
    uint8_t hum_config[2] = {BME280_REG_CTRL_HUM, 0x01};
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, hum_config, sizeof(hum_config), -1));

    // Configurar pressão e temperatura (oversampling x1, modo normal)
    uint8_t meas_config[2] = {BME280_REG_CTRL_MEAS, 0x27};
    ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, meas_config, sizeof(meas_config), -1));
}

void bme280_read_data(i2c_master_dev_handle_t sensorHandle, bme280_data_t *data)
{
    uint8_t reg = BME280_REG_PRESS_MSB;
    uint8_t rawData[8];

    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensorHandle, &reg, sizeof(reg), rawData, sizeof(rawData), -1));

    // Converte os dados brutos (simplificado, sem calibração)
    int32_t adc_P = (rawData[0] << 12) | (rawData[1] << 4) | (rawData[2] >> 4);
    int32_t adc_T = (rawData[3] << 12) | (rawData[4] << 4) | (rawData[5] >> 4);
    int32_t adc_H = (rawData[6] << 8) | rawData[7];

    // Conversão simplificada
    data->pressure = adc_P / 256.0;
    data->temperature = adc_T / 5120.0;
    data->humidity = adc_H / 1024.0;
}
