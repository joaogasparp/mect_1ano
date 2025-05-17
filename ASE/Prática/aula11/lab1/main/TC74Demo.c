#include <stdio.h>
#include "esp_sleep.h"
#include "esp_err.h"
#include "temp_sensor_tc74.h"

// Definições para o sensor TC74
#define TC74_SDA_IO 3
#define TC74_SCL_IO 2
#define TC74_SENSOR_ADDR TC74_A5_SENSOR_ADDR // Utilize o endereço adequado

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;
    i2c_master_dev_handle_t sensorHandle;
    uint8_t temperature;

    // Inicializa o sensor TC74
    tc74_init(&busHandle, &sensorHandle, TC74_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);

    tc74_wakeup(sensorHandle);
    // Leitura inicial de configuração
    tc74_read_temp_after_cfg(sensorHandle, &temperature);

    printf("Temperature: %d ºC\n", temperature);
    printf("A entrar em deep sleep por 3 segundos...\n");

    // Configura deep sleep para 3 segundos (3.000.000 microsegundos)
    esp_sleep_enable_timer_wakeup(3000000);
    esp_deep_sleep_start();
}

// gpio 4 mosi
// gpio 6 miso
// gpio 5 clk
// gpio 1 cs
// alterar isto apenas no menucofngi