#include "led_control.h"
#include "app_config.h"

#include "driver/gpio.h"

static bool led_state = false;

void led_control_init(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);
}

void led_control_set(bool state)
{
    led_state = state;
    gpio_set_level(LED_GPIO, led_state ? 1 : 0);
}

void led_control_toggle(void)
{
    led_control_set(!led_state);
}