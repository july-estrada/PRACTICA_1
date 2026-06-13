#include "adc_reader.h"
#include "app_config.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "ADC";

/* Handle del ADC en modo oneshot */
static adc_oneshot_unit_handle_t adc_handle = NULL;

void adc_reader_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,
                                               ADC_CHANNEL_6,
                                               &channel_config));

    ESP_LOGI(TAG, "ADC inicializado en GPIO34");
}

uint16_t adc_reader_get_raw(void)
{
    int raw = 0;

    if (adc_handle == NULL) {
        return 0;
    }

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &raw));

    return (uint16_t)raw;
}

float adc_reader_raw_to_voltage(uint16_t raw)
{
    return ((float)raw * 3.3f) / 4095.0f;
}