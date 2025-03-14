#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "brightness_level";

#define BLINK_GPIO 10
#define LEDC_FREQUENCY 500
#define PWM_RESOLUTION LEDC_TIMER_8_BIT

static uint8_t brightness_level = 0;

static void update_brightness_level(void)
{
    uint8_t duty = (brightness_level * 1023) / 9;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void pwm_init(void)
{
    // configura o canal do pwm
    ledc_channel_config_t ledc_channel = {
        .gpio_num = BLINK_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    // configura o timer do pwm
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);
}

static void brightness_timer_callback(void *args)
{
    brightness_level = (brightness_level + 1) % 10;
    update_brightness_level();
}

void app_main(void)
{
    // configura o pwm
    pwm_init();

    // atualizar o brilho inicialmente
    update_brightness_level();

    const esp_timer_create_args_t timer_args = {
        .callback = brightness_timer_callback,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "brightness_led",
    };
    esp_timer_handle_t timer_handler;
    esp_timer_create(&timer_args, &timer_handler);

    esp_timer_start_periodic(timer_handler, 2000000);

    while (1)
    {
        // tem de ser e pode ser porque não controla a temporização do timer
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
