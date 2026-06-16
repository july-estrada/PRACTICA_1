#include "led_control.h"
#include "app_config.h"

#include "driver/gpio.h"

/* Variable interna que guarda el estado actual del LED.
   false = apagado, true = encendido. */
static bool led_state = false;

/* Inicializa el GPIO del LED como salida digital. */
void led_control_init(void)
{
    /* Reinicia la configuración previa del pin del LED. */
    gpio_reset_pin(LED_GPIO);

    /* Configura el pin del LED como salida. */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    /* Asegura que el LED inicie apagado. */
    gpio_set_level(LED_GPIO, 0);
}

/* Establece manualmente el estado del LED. */
void led_control_set(bool state)
{
    /* Guarda el nuevo estado del LED en la variable interna. */
    led_state = state;

    /* Envía el valor lógico al GPIO: 1 encendido, 0 apagado. */
    gpio_set_level(LED_GPIO, led_state ? 1 : 0);
}

/* Invierte el estado actual del LED.
   Si está encendido lo apaga, y si está apagado lo enciende. */
void led_control_toggle(void)
{
    led_control_set(!led_state);
}