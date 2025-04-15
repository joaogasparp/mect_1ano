#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "temp_sensor_tc74.h"

#define TC74_SDA_IO 3
#define TC74_SCL_IO 2

#define TEMP_BUFFER_SIZE 3

static uint8_t tempBuffer[TEMP_BUFFER_SIZE] = {0};
static uint8_t tempIndex = 0;
static TimerHandle_t tempTimer;
static i2c_master_dev_handle_t sensorHandle;

void collect_and_print_temperature(TimerHandle_t xTimer)
{
    uint8_t temperature;
    tc74_read_temp_after_cfg(sensorHandle, &temperature);

    tempBuffer[tempIndex] = temperature;
    tempIndex = (tempIndex + 1) % TEMP_BUFFER_SIZE;

    printf("Current Temperature: %d ºC\n", temperature);
}

void check_keypress_and_calculate_average(void *pvParameters)
{
    while (1)
    {
        int c = fgetc(stdin);
        if (c != EOF)
        {
            uint8_t sum = 0;
            for (int i = 0; i < TEMP_BUFFER_SIZE; i++)
            {
                sum += tempBuffer[i];
            }
            uint8_t averageTemp = sum / TEMP_BUFFER_SIZE;

            printf("[Key Pressed] Average Temperature: %d ºC\n", averageTemp);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;

    tc74_init(&busHandle, &sensorHandle, TC74_A5_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);
    tc74_wakeup(sensorHandle);

    tempTimer = xTimerCreate("TempTimer", pdMS_TO_TICKS(1000), pdTRUE, NULL, collect_and_print_temperature);
    if (tempTimer == NULL)
    {
        printf("Failed to create timer\n");
        return;
    }
    xTimerStart(tempTimer, 0);

    xTaskCreate(check_keypress_and_calculate_average, "CheckKeypressTask", 2048, NULL, 5, NULL);
}
