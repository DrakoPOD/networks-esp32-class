#include <Arduino.h>

#include "FreeRTOS.h"
#include "task.h"

struct BlinkParams {
    int pin;
    int delay_ms;
    const char *taskName;
};

void blinkTask(void *pvParameters) {
    BlinkParams *params = (BlinkParams *)pvParameters;
    pinMode(params->pin, OUTPUT);

    while (1) {
        digitalWrite(params->pin, HIGH);
        vTaskDelay(params->delay_ms / portTICK_PERIOD_MS);
        digitalWrite(params->pin, LOW);
        vTaskDelay(params->delay_ms / portTICK_PERIOD_MS);
    }
}

void blink(int pin, int delay_ms, const char *taskName) {
    BlinkParams *params = new BlinkParams{pin, delay_ms, taskName};

    xTaskCreate(
        blinkTask,       // Función de la tarea
        taskName,        // Nombre de la tarea (dinámico)
        4096,            // Tamaño de la pila
        (void *)params,  // Pasar la estructura como argumento
        1,               // Prioridad
        NULL             // Handle de la tarea (opcional)
    );
}