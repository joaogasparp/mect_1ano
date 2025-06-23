#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "temp_sensor_tc74.h"

#define TC74_SDA_IO 3
#define TC74_SCL_IO 2
#define TEMP_BUFFER_SIZE 3

// Estrutura para armazenar temperatura com timestamp
typedef struct
{
    uint8_t temperature;
    uint32_t timestamp; // Timestamp em segundos desde boot
} temp_data_t;

// Variáveis compartilhadas protegidas por mutex
static temp_data_t tempBuffer[TEMP_BUFFER_SIZE] = {0};
static uint8_t tempIndex = 0;
static bool bufferFull = false;
static temp_data_t maxTemp = {0, 0};
static temp_data_t minTemp = {255, 0}; // Inicializar com valor alto
static bool firstReading = true;

// Handles para sincronização
static TimerHandle_t tempTimer;
static SemaphoreHandle_t dataMutex;
static i2c_master_dev_handle_t sensorHandle;

// Função para obter timestamp atual (segundos desde boot)
static uint32_t get_timestamp(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000000); // Converter de microsegundos para segundos
}

// Timer callback - coleta temperatura a cada segundo
void collect_temperature_timer(TimerHandle_t xTimer)
{
    uint8_t temperature;
    tc74_read_temp_after_temp(sensorHandle, &temperature);

    uint32_t timestamp = get_timestamp();

    // Proteger acesso às variáveis compartilhadas
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {

        // Armazenar no buffer circular com timestamp
        tempBuffer[tempIndex].temperature = temperature;
        tempBuffer[tempIndex].timestamp = timestamp;
        tempIndex = (tempIndex + 1) % TEMP_BUFFER_SIZE;

        if (tempIndex == 0)
        {
            bufferFull = true;
        }

        // Atualizar máximo e mínimo
        if (firstReading)
        {
            maxTemp.temperature = temperature;
            maxTemp.timestamp = timestamp;
            minTemp.temperature = temperature;
            minTemp.timestamp = timestamp;
            firstReading = false;
        }
        else
        {
            if (temperature > maxTemp.temperature)
            {
                maxTemp.temperature = temperature;
                maxTemp.timestamp = timestamp;
            }
            if (temperature < minTemp.temperature)
            {
                minTemp.temperature = temperature;
                minTemp.timestamp = timestamp;
            }
        }

        xSemaphoreGive(dataMutex);
    }

    printf("Temperature collected: %d ºC at %lu seconds\n", temperature, timestamp);
}

// Task - verifica tecla pressionada e exibe informações
void keypress_task(void *pvParameters)
{
    while (1)
    {
        int c = fgetc(stdin);
        if (c != EOF)
        {

            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
            {

                switch (c)
                {
                case 'a': // Average
                case 'A':
                    if (bufferFull || tempIndex > 0)
                    {
                        uint32_t sum = 0;
                        uint8_t count = bufferFull ? TEMP_BUFFER_SIZE : tempIndex;
                        uint32_t latest_timestamp = 0;

                        for (int i = 0; i < count; i++)
                        {
                            sum += tempBuffer[i].temperature;
                            if (tempBuffer[i].timestamp > latest_timestamp)
                            {
                                latest_timestamp = tempBuffer[i].timestamp;
                            }
                        }

                        uint8_t averageTemp = sum / count;
                        printf("[AVERAGE] Temperature: %d ºC (last reading at %lu seconds)\n",
                               averageTemp, latest_timestamp);
                    }
                    else
                    {
                        printf("[AVERAGE] No temperature data available yet\n");
                    }
                    break;

                case 'm': // Maximum
                case 'M':
                    if (!firstReading)
                    {
                        printf("[MAXIMUM] Temperature: %d ºC at %lu seconds\n",
                               maxTemp.temperature, maxTemp.timestamp);
                    }
                    else
                    {
                        printf("[MAXIMUM] No temperature data available yet\n");
                    }
                    break;

                case 'n': // Minimum
                case 'N':
                    if (!firstReading)
                    {
                        printf("[MINIMUM] Temperature: %d ºC at %lu seconds\n",
                               minTemp.temperature, minTemp.timestamp);
                    }
                    else
                    {
                        printf("[MINIMUM] No temperature data available yet\n");
                    }
                    break;

                default:
                    printf("Commands: 'A' = Average, 'M' = Maximum, 'N' = Minimum\n");
                    break;
                }

                xSemaphoreGive(dataMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t busHandle;

    printf("TC74 Temperature System with Timestamps and Mutex\n");
    printf("Commands: 'A' = Average, 'M' = Maximum, 'N' = Minimum\n\n");

    // Criar mutex para proteção das variáveis compartilhadas
    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == NULL)
    {
        printf("Failed to create data mutex\n");
        return;
    }

    // Inicializar sensor TC74
    tc74_init(&busHandle, &sensorHandle, TC74_A5_SENSOR_ADDR,
              TC74_SDA_IO, TC74_SCL_IO, TC74_SCL_DFLT_FREQ_HZ);

    // Fazer primeira leitura para configurar o ponteiro do registrador
    tc74_wakeup(sensorHandle);
    vTaskDelay(pdMS_TO_TICKS(250));

    uint8_t temp;
    tc74_read_temp_after_cfg(sensorHandle, &temp);

    // Criar timer para coleta de temperatura (1 segundo)
    tempTimer = xTimerCreate("TempTimer",
                             pdMS_TO_TICKS(1000),
                             pdTRUE,
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
