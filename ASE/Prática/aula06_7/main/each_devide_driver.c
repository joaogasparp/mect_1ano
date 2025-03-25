#include <stdio.h>
#include "esp_err.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "temp_sensor_tc74.h"

#define TC74_SDA_IO 3
#define TC74_SCL_IO 2

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;

    tc74_init(&busHandle, &sensorHandle, TC74_A5_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);

    while (1)
    {
        tc74_wakeup(sensorHandle);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// primeiro wakeup em ciclo infinito depois passa para fora do main
// e fica a tc74_read_temp_after_config em ciclo infinito (podemos
// aquecer o sensor). depois disso essa passa para fora também e a 
// que fica é a tc74_read_temp_after_temp em ciclo infinito também.
// testar a standby é igual à wakeup mas depois dela não há mais.