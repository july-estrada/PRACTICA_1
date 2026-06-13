#include "app_config.h"
#include "adc_reader.h"
#include "led_control.h"
#include "tasks.h"

#include "driver/gpio.h"
#include "esp_log.h"

/**
 * @brief Idle Hook de FreeRTOS.
 *
 * Se ejecuta cuando todas las tareas de usuario están bloqueadas.
 * No debe contener retardos ni llamadas bloqueantes.
 */
void vApplicationIdleHook(void)
{
    static TickType_t last_print = 0;

    if ((xTaskGetTickCount() - last_print) > pdMS_TO_TICKS(2000))
    {
        ESP_LOGI("IDLE", "[IDLE] CPU libre - esperando evento de boton");
        last_print = xTaskGetTickCount();
    }
}

void app_main(void)
{
    ESP_LOGI("MAIN", "=== Practica 1 - Ciclo LED/Sensor FreeRTOS ===");
    ESP_LOGI("MAIN", "Ciclo: rapido 5s -> lento 5s -> idle 5s");

    /* Configurar botón BOOT */
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);


    /* Inicializar periféricos */
    led_control_init();
    adc_reader_init();

    /* Crear tareas */
    tasks_create_all();
}