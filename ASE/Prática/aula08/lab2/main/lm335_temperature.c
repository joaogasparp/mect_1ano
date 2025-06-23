#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "LM335_TEMP";

// Definições do ADC
#define TEMP_ADC_CHANNEL ADC_CHANNEL_2 // GPIO2 no ESP32-C3
#define TEMP_ADC_UNIT ADC_UNIT_1
#define TEMP_ADC_ATTEN ADC_ATTEN_DB_12 // 0-3.3V range (adequado para LM335)

// Constantes do LM335
#define LM335_MV_PER_KELVIN 10.0f // 10mV/K
#define KELVIN_TO_CELSIUS 273.15f // Conversão K para °C
#define FAHRENHEIT_OFFSET 32.0f   // Offset para °F
#define FAHRENHEIT_SCALE 1.8f     // Escala para °F

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
        ESP_LOGW(TAG, "ADC Calibration Failed");
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

// Função para converter voltagem em temperatura
static void voltage_to_temperature(int voltage_mv, float *temp_celsius, float *temp_fahrenheit, float *temp_kelvin)
{
    // LM335: 10mV/K
    *temp_kelvin = voltage_mv / LM335_MV_PER_KELVIN;
    *temp_celsius = *temp_kelvin - KELVIN_TO_CELSIUS;
    *temp_fahrenheit = (*temp_celsius * FAHRENHEIT_SCALE) + FAHRENHEIT_OFFSET;
}

void app_main(void)
{
    ESP_LOGI(TAG, "LM335 Temperature Sensor Application");
    ESP_LOGI(TAG, "Connect LM335: (+) via 2.2kΩ to 3.3V, (-) to GND, middle to GPIO2");

    // Inicializar ADC1
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = TEMP_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // Configurar canal ADC
    adc_oneshot_chan_cfg_t config = {
        .atten = TEMP_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, TEMP_ADC_CHANNEL, &config));

    // Inicializar calibração
    adc_cali_handle_t adc_cali_handle = NULL;
    bool do_calibration = adc_calibration_init(TEMP_ADC_UNIT, TEMP_ADC_CHANNEL, TEMP_ADC_ATTEN, &adc_cali_handle);

    // Mostrar configuração
    ESP_LOGI(TAG, "ADC Configuration:");
    ESP_LOGI(TAG, "- Unit: ADC%d", TEMP_ADC_UNIT + 1);
    ESP_LOGI(TAG, "- Channel: %d (GPIO2)", TEMP_ADC_CHANNEL);
    ESP_LOGI(TAG, "- Attenuation: 12dB (0-3.3V) - Ideal for LM335");
    ESP_LOGI(TAG, "- Calibration: %s", do_calibration ? "Enabled" : "Disabled");
    ESP_LOGI(TAG, "- LM335 Sensitivity: %.1f mV/K", LM335_MV_PER_KELVIN);
    ESP_LOGI(TAG, "Starting temperature readings...\n");

    // Variáveis para temperatura
    float temp_celsius, temp_fahrenheit, temp_kelvin;
    int reading_count = 0;

    // Loop principal de leitura
    while (1)
    {
        // Ler valor bruto do ADC
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, TEMP_ADC_CHANNEL, &adc_raw_value));

        if (do_calibration)
        {
            // Converter para voltagem calibrada
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw_value, &voltage_mv));

            // Converter voltagem para temperatura
            voltage_to_temperature(voltage_mv, &temp_celsius, &temp_fahrenheit, &temp_kelvin);

            // Mostrar resultados completos
            printf("[%04d] Raw=%4d | Voltage=%4d mV | %.1f K | %.2f °C | %.2f °F\n",
                   ++reading_count,
                   adc_raw_value,
                   voltage_mv,
                   temp_kelvin,
                   temp_celsius,
                   temp_fahrenheit);

            // Verificar se a leitura está na faixa esperada
            if (voltage_mv < 2500 || voltage_mv > 3200)
            {
                ESP_LOGW(TAG, "Warning: Voltage out of typical LM335 range (2500-3200mV)");
            }
        }
        else
        {
            // Mostrar apenas valor bruto sem calibração
            printf("[%04d] Raw=%4d (Calibration not available)\n",
                   ++reading_count, adc_raw_value);
        }

        // Aguardar 1 segundo antes da próxima leitura
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Limpeza (nunca executado neste exemplo)
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibration)
    {
        adc_calibration_deinit(adc_cali_handle);
    }
}
