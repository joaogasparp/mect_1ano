#include <stdio.h>
#include "esp_sleep.h"
#include "esp_err.h"
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

    tc74_wakeup(sensorHandle);
    tc74_read_temp_after_cfg(sensorHandle, &temperature);

    tc74_read_temp_after_temp(sensorHandle, &temperature);
    printf("Temperature: %d ºC\n", temperature);

    esp_sleep_enable_timer_wakeup(3000000); // 3 segundos
    esp_deep_sleep_start();
}
