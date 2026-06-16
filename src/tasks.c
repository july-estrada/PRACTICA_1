#include "tasks.h"
#include "app_config.h"
#include "led_control.h"
#include "adc_reader.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "TASKS";

volatile AppState_t g_appState = APP_FAST;
volatile bool g_buttonPressed = false;

TaskHandle_t hLedRapido = NULL;
TaskHandle_t hLedLento = NULL;
TaskHandle_t hSensor = NULL;
TaskHandle_t hMonitor = NULL;

void vTaskLedRapido(void *pvParameters)
{
    // Se evita advertencia de parametro no utilizado
    (void)pvParameters;

    for (;;)
    {
        if (g_appState == APP_FAST) // La tarea solo trabaja cuando el sistema esta en modo rapido
        {
            ESP_LOGI(TAG, "[LED_R] Iniciando parpadeo rapido por 5 segundos");

            TickType_t start_tick = xTaskGetTickCount(); // Guarda el tiempo de inicio

            // Mantiene el parpadeo durante FAST_TOTAL_MS
            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < FAST_TOTAL_MS)
            {
                // Cambia el estado del LED
                led_control_toggle();

                // Muestra el tick actual
                ESP_LOGI(TAG, "[LED_R] tick:%lu", (unsigned long)xTaskGetTickCount());
                // Espera el periodo de parpadeo rapido
                vTaskDelay(pdMS_TO_TICKS(LED_FAST_MS));
            }

            // Apaga el LED al terminar
            led_control_set(false);
            // Cambia al siguiente estado
            g_appState = APP_SLOW;

            ESP_LOGI(TAG, "[LED_R] Fin de modo rapido -> modo lento");
        }

        // Cede tiempo al scheduler
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* Tarea encargada del parpadeo lento del LED.
   Se ejecuta únicamente cuando el estado global del sistema es APP_SLOW. */
void vTaskLedLento(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        // La tarea solo realiza acciones si el sistema está en modo lento. 
        if (g_appState == APP_SLOW)
        {
            ESP_LOGI(TAG, "[LED_L] Iniciando parpadeo lento por 5 segundos");

            // Guarda el tick inicial para medir cuánto tiempo dura el modo lento.
            TickType_t start_tick = xTaskGetTickCount();

             // Mantiene el parpadeo lento durante SLOW_TOTAL_MS.
            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < SLOW_TOTAL_MS)
            {
                // Cambia el estado del LED entre encendido y apagado.
                led_control_toggle();
                 // Imprime el tick actual para evidenciar la ejecución de la tarea.
                ESP_LOGI(TAG, "[LED_L] tick:%lu", (unsigned long)xTaskGetTickCount());
                // Retardo correspondiente al parpadeo lento.
                vTaskDelay(pdMS_TO_TICKS(LED_SLOW_MS));
            }

            // Al terminar el modo lento se apaga el LED.
            led_control_set(false);
            // Limpia la bandera del botón antes de entrar al estado de espera.
            g_buttonPressed = false;
            // Cambia el sistema al estado de espera/Idle.
            g_appState = APP_IDLE_WAIT;

            ESP_LOGI(TAG, "[LED_L] Fin de modo lento -> espera/Idle");
        }

        // Pequeño retardo para no saturar el CPU cuando la tarea no está activa.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* Tarea encargada de leer el ADC cuando el sistema está en espera.
   Solo realiza la lectura si se presiona el botón BOOT durante APP_IDLE_WAIT. */
void vTaskSensor(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        // La lectura del ADC solo está habilitada durante el estado de espera.
        if (g_appState == APP_IDLE_WAIT)
        {
            // Guarda el tick inicial para contar los 5 segundos de espera.
            TickType_t start_tick = xTaskGetTickCount();


            ESP_LOGI(TAG, "[SENS] Presiona BOOT para leer ADC y regresar a rapido");

            // El sistema permanece en espera durante IDLE_WAIT_MS o hasta que se presione el botón.
            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < IDLE_WAIT_MS)
            {
                // Si el monitor detectó el botón, se realiza la lectura del potenciómetro. 
                if (g_buttonPressed)
                {
                     // Lectura cruda del ADC.
                    uint16_t raw = adc_reader_get_raw();
                    float voltage = adc_reader_raw_to_voltage(raw);

                    // Impresión del valor ADC, voltaje y tick actual.
                    ESP_LOGI(TAG, "[SENS] ADC raw:%u  %.2fV  tick:%lu",
                             raw,
                             voltage,
                             (unsigned long)xTaskGetTickCount());

                    // Limpia la bandera del botón para evitar lecturas repetidas
                    g_buttonPressed = false;
                    // Regresa inmediatamente al modo de parpadeo rápido.
                    g_appState = APP_FAST;

                    ESP_LOGI(TAG, "[SENS] Boton detectado -> regreso inmediato a modo rapido");
                    break;
                }

                // Retardo corto para revisar el botón sin saturar.
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            // Si no se presionó el botón dentro de 5 segundos, el sistema reinicia el ciclo.
            if (g_appState == APP_IDLE_WAIT)
            {
                ESP_LOGI(TAG, "[IDLE] Timeout 5s sin boton -> regreso a modo rapido");
                g_appState = APP_FAST;
            }
        }

        // Periodo general de revisión de la tarea sensor.
        vTaskDelay(pdMS_TO_TICKS(SENSOR_MS));
    }
}

/* Tarea de monitoreo del botón BOOT y del estado general del sistema.
   Detecta el flanco de bajada del botón y reporta información por UART. */
void vTaskMonitor(void *pvParameters)
{
    (void)pvParameters;

     // Estado anterior del botón. Con pull-up, 1 significa no presionado.
    int last_button = 1;

    for (;;)
    {
        // Lee el estado actual del botón BOOT.
        int current_button = gpio_get_level(BUTTON_GPIO);

        // Detecta flanco de bajada: antes estaba en 1 y ahora está en 0.
        if ((last_button == 1) && (current_button == 0))
        {
            // El botón solo tiene efecto durante el estado de espera.
            if (g_appState == APP_IDLE_WAIT)
            {
                // Activa bandera para que vTaskSensor realice la lectura ADC
                g_buttonPressed = true;
                ESP_LOGI(TAG, "[MON] *** BOTON PRESIONADO EN IDLE ***");
            }
            else
            {
                // Si el sistema no está en espera, el botón se ignora.
                ESP_LOGI(TAG, "[MON] Boton ignorado, el sistema no esta en Idle");
            }
        }

        // Actualiza el estado anterior del botón para la siguiente iteración.
        last_button = current_button;

         // Imprime información de memoria heap, stack libre de tareas y estado actual.
        ESP_LOGI(TAG,
                 "[MON] Heap:%lu Stack_R:%u Stack_L:%u Stack_S:%u Stack_M:%u Estado:%d",
                 esp_get_free_heap_size(),
                 uxTaskGetStackHighWaterMark(hLedRapido),
                 uxTaskGetStackHighWaterMark(hLedLento),
                 uxTaskGetStackHighWaterMark(hSensor),
                 uxTaskGetStackHighWaterMark(hMonitor),
                 g_appState);

        // Periodo de monitoreo del sistema.
        vTaskDelay(pdMS_TO_TICKS(MONITOR_MS));
    }
}

/* Crea todas las tareas utilizadas en la práctica.
   Cada tarea recibe un nombre, tamaño de stack, prioridad y handle correspondiente. */
void tasks_create_all(void)
{
     // Tarea de parpadeo rápido del LED. Prioridad 1.
    xTaskCreate(vTaskLedRapido, "LED_RAPIDO", 2048, NULL, 1, &hLedRapido);
    // Tarea de parpadeo lento del LED. Prioridad 2.
    xTaskCreate(vTaskLedLento,  "LED_LENTO",  2048, NULL, 2, &hLedLento);
    // Tarea encargada de leer el ADC durante el estado de espera. Prioridad 3.
    xTaskCreate(vTaskSensor,    "SENSOR",     2048, NULL, 3, &hSensor);
    // Tarea de monitoreo del botón y del estado del sistema. Prioridad 4.
    xTaskCreate(vTaskMonitor,   "MONITOR",    3072, NULL, 4, &hMonitor);

    ESP_LOGI(TAG, "Tareas creadas correctamente");
}