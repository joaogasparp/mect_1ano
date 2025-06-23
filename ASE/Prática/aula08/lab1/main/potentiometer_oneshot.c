#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "POTENTIOMETER";

// Definições do ADC
#define POT_ADC_CHANNEL ADC_CHANNEL_2 // GPIO2 no ESP32-C3
#define POT_ADC_UNIT ADC_UNIT_1
#define POT_ADC_ATTEN ADC_ATTEN_DB_12 // 0-3.3V range

// Variáveis globais
static int adc_raw_value = 0;
static int voltage_mv = 0;

// Função para inicializar calibração do ADC
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "Calibration scheme: Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "Calibration scheme: Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "ADC Calibration Success");
    }
    else
    {
        ESP_LOGW(TAG, "ADC Calibration Failed - using raw values only");
    }

    return calibrated;
}

// Função para destruir calibração
static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

void app_main(void)
{
    ESP_LOGI(TAG, "Potentiometer ADC Reading Application");
    ESP_LOGI(TAG, "Connect 10k potentiometer: VCC->3.3V, Wiper->GPIO2, GND->GND");

    // Inicializar ADC1
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = POT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // Configurar canal ADC
    adc_oneshot_chan_cfg_t config = {
        .atten = POT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, POT_ADC_CHANNEL, &config));

    // Inicializar calibração
    adc_cali_handle_t adc_cali_handle = NULL;
    bool do_calibration = adc_calibration_init(POT_ADC_UNIT, POT_ADC_CHANNEL, POT_ADC_ATTEN, &adc_cali_handle);

    // Mostrar configuração atual
    ESP_LOGI(TAG, "ADC Configuration:");
    ESP_LOGI(TAG, "- Unit: ADC%d", POT_ADC_UNIT + 1);
    ESP_LOGI(TAG, "- Channel: %d (GPIO2)", POT_ADC_CHANNEL);
    ESP_LOGI(TAG, "- Attenuation: %s",
             (POT_ADC_ATTEN == ADC_ATTEN_DB_0) ? "0dB (0-1.1V)" : (POT_ADC_ATTEN == ADC_ATTEN_DB_2_5) ? "2.5dB (0-1.5V)"
                                                              : (POT_ADC_ATTEN == ADC_ATTEN_DB_6)     ? "6dB (0-2.2V)"
                                                              : (POT_ADC_ATTEN == ADC_ATTEN_DB_12)    ? "12dB (0-3.3V)"
                                                                                                      : "Unknown");
    ESP_LOGI(TAG, "- Calibration: %s", do_calibration ? "Enabled" : "Disabled");
    ESP_LOGI(TAG, "Starting readings...\n");

    // Loop principal de leitura
    while (1)
    {
        // Ler valor bruto do ADC
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, POT_ADC_CHANNEL, &adc_raw_value));

        // Converter para voltagem se calibração disponível
        if (do_calibration)
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw_value, &voltage_mv));

            // Mostrar resultados com calibração
            printf("Potentiometer: Raw=%4d | Voltage=%4d mV | %.3f V | %.1f%%\n",
                   adc_raw_value,
                   voltage_mv,
                   voltage_mv / 1000.0,
                   (voltage_mv / 3300.0) * 100.0);
        }
        else
        {
            // Mostrar apenas valor bruto
            printf("Potentiometer: Raw=%4d (Calibration not available)\n", adc_raw_value);
        }

        // Aguardar 500ms antes da próxima leitura
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Limpeza (nunca executado neste exemplo)
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibration)
    {
        adc_calibration_deinit(adc_cali_handle);
    }
}
