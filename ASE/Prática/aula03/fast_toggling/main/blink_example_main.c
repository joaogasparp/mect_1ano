#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "fast_toggiling";

#define BLINK_GPIO 10
#define CONFIG_BLINK_PERIOD_MS 10

static void blink_led(uint8_t s_led_state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

/*void configure_uart(void)
{
    const uart_port_t uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };
    uart_param_config(uart_num, &uart_config);
    uart_driver_install(uart_num, 1024 * 2, 0, 0, NULL, 0);
}*/

void app_main(void)
{
    static uint8_t s_led_state = 0;

    configure_led();
    // configure_uart();

    while (1) {
        blink_led(s_led_state);
        s_led_state = !s_led_state;

        vTaskDelay(CONFIG_BLINK_PERIOD_MS / portTICK_PERIOD_MS);
    }
}
