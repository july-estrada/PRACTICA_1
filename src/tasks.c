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
    (void)pvParameters;

    for (;;)
    {
        if (g_appState == APP_FAST)
        {
            ESP_LOGI(TAG, "[LED_R] Iniciando parpadeo rapido por 5 segundos");

            TickType_t start_tick = xTaskGetTickCount();

            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < FAST_TOTAL_MS)
            {
                led_control_toggle();
                ESP_LOGI(TAG, "[LED_R] tick:%lu", (unsigned long)xTaskGetTickCount());
                vTaskDelay(pdMS_TO_TICKS(LED_FAST_MS));
            }

            led_control_set(false);
            g_appState = APP_SLOW;

            ESP_LOGI(TAG, "[LED_R] Fin de modo rapido -> modo lento");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void vTaskLedLento(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        if (g_appState == APP_SLOW)
        {
            ESP_LOGI(TAG, "[LED_L] Iniciando parpadeo lento por 5 segundos");

            TickType_t start_tick = xTaskGetTickCount();

            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < SLOW_TOTAL_MS)
            {
                led_control_toggle();
                ESP_LOGI(TAG, "[LED_L] tick:%lu", (unsigned long)xTaskGetTickCount());
                vTaskDelay(pdMS_TO_TICKS(LED_SLOW_MS));
            }

            led_control_set(false);
            g_buttonPressed = false;
            g_appState = APP_IDLE_WAIT;

            ESP_LOGI(TAG, "[LED_L] Fin de modo lento -> espera/Idle");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void vTaskSensor(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        if (g_appState == APP_IDLE_WAIT)
        {
            TickType_t start_tick = xTaskGetTickCount();

            
            ESP_LOGI(TAG, "[SENS] Presiona BOOT para leer ADC y regresar a rapido");

            while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < IDLE_WAIT_MS)
            {
                if (g_buttonPressed)
                {
                    uint16_t raw = adc_reader_get_raw();
                    float voltage = adc_reader_raw_to_voltage(raw);

                    ESP_LOGI(TAG, "[SENS] ADC raw:%u  %.2fV  tick:%lu",
                             raw,
                             voltage,
                             (unsigned long)xTaskGetTickCount());

                    g_buttonPressed = false;
                    g_appState = APP_FAST;

                    ESP_LOGI(TAG, "[SENS] Boton detectado -> regreso inmediato a modo rapido");
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(50));
            }

            if (g_appState == APP_IDLE_WAIT)
            {
                ESP_LOGI(TAG, "[IDLE] Timeout 5s sin boton -> regreso a modo rapido");
                g_appState = APP_FAST;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_MS));
    }
}

void vTaskMonitor(void *pvParameters)
{
    (void)pvParameters;

    int last_button = 1;

    for (;;)
    {
        int current_button = gpio_get_level(BUTTON_GPIO);

        if ((last_button == 1) && (current_button == 0))
        {
            if (g_appState == APP_IDLE_WAIT)
            {
                g_buttonPressed = true;
                ESP_LOGI(TAG, "[MON] *** BOTON PRESIONADO EN IDLE ***");
            }
            else
            {
                ESP_LOGI(TAG, "[MON] Boton ignorado, el sistema no esta en Idle");
            }
        }

        last_button = current_button;

        ESP_LOGI(TAG,
                 "[MON] Heap:%lu Stack_R:%u Stack_L:%u Stack_S:%u Stack_M:%u Estado:%d",
                 esp_get_free_heap_size(),
                 uxTaskGetStackHighWaterMark(hLedRapido),
                 uxTaskGetStackHighWaterMark(hLedLento),
                 uxTaskGetStackHighWaterMark(hSensor),
                 uxTaskGetStackHighWaterMark(hMonitor),
                 g_appState);

        vTaskDelay(pdMS_TO_TICKS(MONITOR_MS));
    }
}

void tasks_create_all(void)
{
    xTaskCreate(vTaskLedRapido, "LED_RAPIDO", 2048, NULL, 1, &hLedRapido);
    xTaskCreate(vTaskLedLento,  "LED_LENTO",  2048, NULL, 2, &hLedLento);
    xTaskCreate(vTaskSensor,    "SENSOR",     2048, NULL, 3, &hSensor);
    xTaskCreate(vTaskMonitor,   "MONITOR",    3072, NULL, 4, &hMonitor);

    ESP_LOGI(TAG, "Tareas creadas correctamente");
}