#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#define I2C_SDA_IO 21
#define I2C_SCL_IO 22
#define I2C_FREQ_HZ 100000
#define DUMMY_I2C_ADDR 0x50 // Dummy address for demonstration

void app_main(void)
{
    // I2C master bus configuration
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    // I2C device configuration
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DUMMY_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    uint8_t tx_data[] = {0x12, 0x34, 0x56, 0x78}; // Test data

    while (1)
    {
        // Attempt to transmit data (will generate START, address, data, STOP)
        esp_err_t ret = i2c_master_transmit(dev_handle, tx_data, sizeof(tx_data), 1000);

        if (ret == ESP_OK)
        {
            printf("I2C transmitted successfully\n");
        }
        else
        {
            printf("I2C transmission failed (expected - no device at address 0x%02X)\n", DUMMY_I2C_ADDR);
        }

        // Wait 1 second before next transmission
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
