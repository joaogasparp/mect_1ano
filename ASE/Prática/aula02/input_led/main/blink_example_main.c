#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "button_led";

#define LED_GPIO 10
#define BUTTON_GPIO 0

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);

    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

    while (1) {
        int button_state = gpio_get_level(BUTTON_GPIO);

        gpio_set_level(LED_GPIO, !button_state);

        ESP_LOGI(TAG, "Botão: %s | LED: %s",
                 button_state ? "Solto" : "Pressionado",
                 button_state ? "OFF" : "ON");

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
