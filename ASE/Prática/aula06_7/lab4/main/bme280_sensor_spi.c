#include "bme280_sensor_spi.h"
#include "esp_log.h"

static const char *TAG = "BME280_SPI";

void bme280_spi_init(spi_device_handle_t *pSensorHandle,
                     int mosiPin, int misoPin, int sclkPin, int csPin,
                     uint32_t clkSpeedHz)
{
    // Configuração do bus SPI
    spi_bus_config_t busCfg = {
        .mosi_io_num = mosiPin,
        .miso_io_num = misoPin,
        .sclk_io_num = sclkPin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &busCfg, SPI_DMA_CH_AUTO));

    // Configuração do dispositivo BME280
    spi_device_interface_config_t devCfg = {
        .clock_speed_hz = clkSpeedHz,
        .mode = 0, // SPI mode 0
        .spics_io_num = csPin,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devCfg, pSensorHandle));
}

void bme280_spi_free(spi_device_handle_t sensorHandle)
{
    ESP_ERROR_CHECK(spi_bus_remove_device(sensorHandle));
    ESP_ERROR_CHECK(spi_bus_free(SPI2_HOST));
}

void bme280_spi_reset(spi_device_handle_t sensorHandle)
{
    uint8_t txData[2] = {BME280_REG_RESET & 0x7F, 0xB6}; // MSB=0 para escrita

    spi_transaction_t trans = {
        .length = 16, // 2 bytes * 8 bits
        .tx_buffer = txData,
        .rx_buffer = NULL,
    };

    ESP_ERROR_CHECK(spi_device_transmit(sensorHandle, &trans));
}

uint8_t bme280_spi_read_id(spi_device_handle_t sensorHandle)
{
    uint8_t txData = BME280_REG_ID | 0x80; // MSB=1 para leitura
    uint8_t rxData[2] = {0};

    spi_transaction_t trans = {
        .length = 16, // 2 bytes * 8 bits
        .tx_buffer = &txData,
        .rx_buffer = rxData,
        .rxlength = 16,
    };

    ESP_ERROR_CHECK(spi_device_transmit(sensorHandle, &trans));
    return rxData[1]; // O primeiro byte é dummy, o segundo é o ID
}

void bme280_spi_config_sensor(spi_device_handle_t sensorHandle)
{
    // Configurar humidade (oversampling x1)
    uint8_t hum_config[2] = {BME280_REG_CTRL_HUM & 0x7F, 0x01};
    spi_transaction_t trans1 = {
        .length = 16,
        .tx_buffer = hum_config,
        .rx_buffer = NULL,
    };
    ESP_ERROR_CHECK(spi_device_transmit(sensorHandle, &trans1));

    // Configurar pressão e temperatura (oversampling x1, modo normal)
    uint8_t meas_config[2] = {BME280_REG_CTRL_MEAS & 0x7F, 0x27};
    spi_transaction_t trans2 = {
        .length = 16,
        .tx_buffer = meas_config,
        .rx_buffer = NULL,
    };
    ESP_ERROR_CHECK(spi_device_transmit(sensorHandle, &trans2));
}

void bme280_spi_read_data(spi_device_handle_t sensorHandle, bme280_data_t *data)
{
    uint8_t txData = BME280_REG_PRESS_MSB | 0x80; // MSB=1 para leitura
    uint8_t rxData[9] = {0};                      // 1 byte dummy + 8 bytes de dados

    spi_transaction_t trans = {
        .length = 72, // 9 bytes * 8 bits
        .tx_buffer = &txData,
        .rx_buffer = rxData,
        .rxlength = 72,
    };

    ESP_ERROR_CHECK(spi_device_transmit(sensorHandle, &trans));

    // Os dados começam no índice 1 (primeiro byte é dummy)
    uint8_t *rawData = &rxData[1];

    // Converte os dados brutos (simplificado, sem calibração)
    int32_t adc_P = (rawData[0] << 12) | (rawData[1] << 4) | (rawData[2] >> 4);
    int32_t adc_T = (rawData[3] << 12) | (rawData[4] << 4) | (rawData[5] >> 4);
    int32_t adc_H = (rawData[6] << 8) | rawData[7];

    // Conversão simplificada
    data->pressure = adc_P / 256.0;
    data->temperature = adc_T / 5120.0;
    data->humidity = adc_H / 1024.0;
}
