#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_system.h"

#define LED_GPIO 10      // GPIO onde o LED está conectado
#define PWM_FREQ 5000   // Frequência do PWM (5 kHz)
#define PWM_RESOLUTION LEDC_TIMER_8_BIT // Resolução do PWM (8 bits)
#define PWM_MAX_DUTY 255 // Valor máximo do duty cycle (100% de brilho)

static const char *TAG = "led_brightness";

void pwm_init(void)
{
    // Configura o timer do PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // Modo de velocidade baixa
        .timer_num = LEDC_TIMER_0,         // Timer 0
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,               // Frequência do PWM
        .clk_cfg = LEDC_AUTO_CLK,          // Utiliza o clock interno
    };
    ledc_timer_config(&ledc_timer);

    // Configura o canal do PWM
    ledc_channel_config_t ledc_channel = {
        .gpio_num = LED_GPIO,              // GPIO do LED
        .speed_mode = LEDC_LOW_SPEED_MODE, // Modo de velocidade baixa
        .channel = LEDC_CHANNEL_0,         // Canal 0
        .intr_type = LEDC_INTR_DISABLE,    // Sem interrupção
        .timer_sel = LEDC_TIMER_0,         // Seleciona o timer 0
        .duty = 0,                         // Duty cycle inicial (0%)
        .hpoint = 0,                       
    };
    ledc_channel_config(&ledc_channel);
}

void app_main(void)
{
    pwm_init();

    while (1) {
        for (int duty = 0; duty <= PWM_MAX_DUTY; duty++) {
            // Aumenta o brilho do LED de 0% até 100%
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            ESP_LOGI(TAG, "Brilho do LED: %d%%", (duty * 100) / PWM_MAX_DUTY); // Mostra o valor do duty cycle
            vTaskDelay(50 / portTICK_PERIOD_MS); // Delay para dar tempo ao LED de mudar
        }

        vTaskDelay(500 / portTICK_PERIOD_MS); // Pausa de meio segundo

        for (int duty = PWM_MAX_DUTY; duty >= 0; duty--) {
            // Diminui o brilho do LED de 100% até 0%
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            ESP_LOGI(TAG, "Brilho do LED: %d%%", (duty * 100) / PWM_MAX_DUTY); // Mostra o valor do duty cycle
            vTaskDelay(50 / portTICK_PERIOD_MS); // Delay para dar tempo ao LED de mudar
        }
    }
}
