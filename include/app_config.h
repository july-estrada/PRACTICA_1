#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/gpio.h"

/* Pines principales de la práctica */
#define LED_GPIO        GPIO_NUM_2
#define BUTTON_GPIO     GPIO_NUM_0
#define ADC_GPIO        GPIO_NUM_34

/* Tiempos de parpadeo */
#define FAST_TOTAL_MS       5000
#define SLOW_TOTAL_MS       5000
#define IDLE_WAIT_MS        5000

#define LED_FAST_MS         100
#define LED_SLOW_MS         500
#define SENSOR_MS           300
#define MONITOR_MS          1000

#endif

