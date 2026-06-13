#ifndef TASKS_H
#define TASKS_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum
{
    APP_FAST = 0,
    APP_SLOW,
    APP_IDLE_WAIT
} AppState_t;


/* Variables compartidas entre tareas */
extern volatile AppState_t g_appState;
extern volatile bool g_buttonPressed;

/* Handles de tareas */
extern TaskHandle_t hLedRapido;
extern TaskHandle_t hLedLento;
extern TaskHandle_t hSensor;
extern TaskHandle_t hMonitor;

/* Funciones de tarea */
void vTaskLedRapido(void *pvParameters);
void vTaskLedLento(void *pvParameters);
void vTaskSensor(void *pvParameters);
void vTaskMonitor(void *pvParameters);

/* Crea todas las tareas */
void tasks_create_all(void);

#endif