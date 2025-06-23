#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define PIN_NUM_MOSI 23
#define PIN_NUM_MISO 19
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

void app_main(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000, // 1 MHz for easy visualization
        .mode = 0,                 // SPI mode 0
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
    };

    spi_device_handle_t spi;

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Attach the device to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    uint8_t tx_data[] = {0xAA, 0x55, 0xCC, 0x33}; // Test pattern
    uint8_t rx_data[4];

    spi_transaction_t trans = {
        .length = 32, // 4 bytes * 8 bits
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    while (1)
    {
        // Transmit data
        ESP_ERROR_CHECK(spi_device_transmit(spi, &trans));

        printf("SPI transmitted: 0x%02X 0x%02X 0x%02X 0x%02X\n",
               tx_data[0], tx_data[1], tx_data[2], tx_data[3]);

        // Wait 1 second before next transmission
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
