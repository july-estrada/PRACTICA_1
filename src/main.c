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
    // Guarda el último instante en el que se imprimió el mensaje del Idle Hook.
    static TickType_t last_print = 0;

    // Imprime cada 2 segundos para no saturar la UART
    if ((xTaskGetTickCount() - last_print) > pdMS_TO_TICKS(2000))
    {
        ESP_LOGI("IDLE", "[IDLE] CPU libre - esperando evento de boton");
        // Actualiza el tiempo de referencia para el siguiente mensaje.
        last_print = xTaskGetTickCount();
    }
}

/* Secuencia principal: rápido 5 s, lento 5 s y espera con lectura ADC por botón. */
void app_main(void)
{
    ESP_LOGI("MAIN", "=== Practica 1 - Ciclo LED/Sensor FreeRTOS ===");
    ESP_LOGI("MAIN", "Ciclo: rapido 5s -> lento 5s -> idle 5s");

    /* Configura el botón BOOT como entrada digital.
       Se usa pull-up interno, por lo que:
       - sin presionar = 1
       - presionado    = 0 */
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);


    /* Inicializar periféricos */
    // Inicializa el GPIO del LED utilizado para los parpadeos.
    led_control_init();
    adc_reader_init();

    
    /* Crea las tareas FreeRTOS:
       - LED rápido
       - LED lento
       - Sensor ADC
       - Monitor del botón y estado del sistema */
    tasks_create_all();
}