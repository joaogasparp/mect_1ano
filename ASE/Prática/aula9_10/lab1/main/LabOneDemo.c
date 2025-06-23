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
static bool bufferFull = false;
static TimerHandle_t tempTimer;
static i2c_master_dev_handle_t sensorHandle;

// Timer callback - coleta temperatura a cada segundo
void collect_temperature_timer(TimerHandle_t xTimer)
{
    uint8_t temperature;
    tc74_read_temp_after_temp(sensorHandle, &temperature);

    // Armazenar no buffer circular
    tempBuffer[tempIndex] = temperature;
    tempIndex = (tempIndex + 1) % TEMP_BUFFER_SIZE;

    if (tempIndex == 0)
    {
        bufferFull = true; // Buffer foi preenchido completamente
    }

    printf("Temperature collected: %d ºC\n", temperature);
}

// Task - verifica tecla pressionada e calcula média
void keypress_task(void *pvParameters)
{
    while (1)
    {
        int c = fgetc(stdin);
        if (c != EOF)
        {
            // Calcular média dos últimos 3 valores
            if (bufferFull || tempIndex > 0)
            {
                uint16_t sum = 0;
                uint8_t count = bufferFull ? TEMP_BUFFER_SIZE : tempIndex;

                for (int i = 0; i < count; i++)
                {
                    sum += tempBuffer[i];
                }

                uint8_t averageTemp = sum / count;
                printf("[KEY PRESSED] Average of last %d temperatures: %d ºC\n",
                       count, averageTemp);
            }
            else
            {
                printf("[KEY PRESSED] No temperature data available yet\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;

    printf("TC74 Temperature System with Timer and Task\n");
    printf("Press any key to see average temperature\n\n");

    // Inicializar sensor TC74
    tc74_init(&busHandle, &sensorHandle, TC74_A5_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);

    // Fazer primeira leitura para configurar o ponteiro do registrador
    tc74_wakeup(sensorHandle);
    vTaskDelay(pdMS_TO_TICKS(250)); // Aguardar estabilização

    uint8_t temp;
    tc74_read_temp_after_cfg(sensorHandle, &temp);

    // Criar timer para coleta de temperatura (1 segundo)
    tempTimer = xTimerCreate("TempTimer",
                             pdMS_TO_TICKS(1000), // 1 segundo
                             pdTRUE,              // Auto-reload
                             NULL,
                             collect_temperature_timer);

    if (tempTimer == NULL)
    {
        printf("Failed to create temperature timer\n");
        return;
    }

    // Criar task para verificar tecla pressionada
    if (xTaskCreate(keypress_task, "KeypressTask", 2048, NULL, 5, NULL) != pdPASS)
    {
        printf("Failed to create keypress task\n");
        return;
    }

    // Iniciar timer
    if (xTimerStart(tempTimer, 0) != pdPASS)
    {
        printf("Failed to start temperature timer\n");
        return;
    }

    printf("System started successfully!\n");
}
