#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "temp_sensor_tc74.h"

#define TC74_SDA_IO 3
#define TC74_SCL_IO 2

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;
    uint8_t temperature;

    tc74_init(&busHandle, &sensorHandle, TC74_A5_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);

    while (1)
    {
        tc74_wakeup(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    /*
    tc74_wakeup(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1)
    {
        tc74_read_temp_after_cfg(sensorHandle, &temperature);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    tc74_wakeup(sensorHandle);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    tc74_read_temp_after_cfg(sensorHandle, &temperature);

    while (1)
    {
        tc74_read_temp_after_temp(sensorHandle, &temperature);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */

    /*
    while (1)
    {
        tc74_standy(sensorHandle);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    */
}
