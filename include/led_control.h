#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdbool.h>

/* Inicializa el GPIO del LED */
void led_control_init(void);

/* Cambia el estado del LED: encendido o apagado */
void led_control_set(bool state);

/* Invierte el estado actual del LED */
void led_control_toggle(void);

#endif