#include "adc_reader.h"
#include "app_config.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

/* Etiqueta para identificar en UART los mensajes relacionados con el ADC. */
static const char *TAG = "ADC";

/* Handle global del ADC en modo oneshot.
   Permite inicializar el ADC una vez y reutilizarlo en las lecturas posteriores. */
static adc_oneshot_unit_handle_t adc_handle = NULL;

/* Inicializa el ADC1 del ESP32 y configura el canal correspondiente a GPIO34. */
void adc_reader_init(void)
{
    /* Configuración de la unidad ADC.
       Se usa ADC_UNIT_1 porque GPIO34 pertenece al ADC1. */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    /* Crea la unidad ADC en modo oneshot y guarda el handle. */
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    /* Configuración del canal ADC.
       ADC_ATTEN_DB_12 permite leer un rango aproximado de 0 a 3.3 V. */
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    /* GPIO34 corresponde al canal ADC_CHANNEL_6 del ADC1. */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,
                                               ADC_CHANNEL_6,
                                               &channel_config));

    ESP_LOGI(TAG, "ADC inicializado en GPIO34");
}

/* Lee el valor crudo del ADC.
   El valor esperado está aproximadamente entre 0 y 4095 para una resolución de 12 bits. */
uint16_t adc_reader_get_raw(void)
{
    int raw = 0;

    /* Si el ADC no fue inicializado correctamente, se regresa 0 para evitar errores. */
    if (adc_handle == NULL) {
        return 0;
    }

    /* Realiza una lectura del canal ADC asociado a GPIO34. */
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &raw));

    return (uint16_t)raw;
}

/* Convierte el valor crudo del ADC a un voltaje aproximado.
   Se considera un rango de 0 a 3.3 V y resolución de 12 bits: 0 a 4095. */
float adc_reader_raw_to_voltage(uint16_t raw)
{
    return ((float)raw * 3.3f) / 4095.0f;
}