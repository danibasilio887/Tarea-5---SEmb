#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define EX_UART_NUM UART_NUM_2
#define TX_PIN 17
#define RX_PIN 16
#define LED_PIN 2

SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xMutex;
int eventos_totales = 0; // Recurso compartido

#ifdef __cplusplus
extern "C" {
#endif

// --- TAREA 1: PROCESADOR (Acción por Semáforo) ---
void vTaskProcessor(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
            // Sección crítica protegida por Mutex
            xSemaphoreTake(xMutex, portMAX_DELAY);
            eventos_totales++;
            xSemaphoreGive(xMutex);

            // Acción física (Foco)
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500)); // Prendido medio segundo
            gpio_set_level(LED_PIN, 0);
        }
    }
}

// --- TAREA 2: MONITOR (Reporte Periódico) ---
// Esta tarea evita que la terminal se sienta "trabada"
void vTaskReport(void *pvParameters) {
    while (1) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        int copia_eventos = eventos_totales;
        xSemaphoreGive(xMutex);

        char info[50];
        int n = sprintf(info, "\r\n[ESTADO] Eventos acumulados: %d\r\n", copia_eventos);
        uart_write_bytes(EX_UART_NUM, info, n);

        vTaskDelay(pdMS_TO_TICKS(3000)); // Reporta cada 3 segundos
    }
}

// --- TAREA 3: UART (Recepción) ---
void vTaskUART(void *pvParameters) {
    uint8_t ch;
    while (1) {
        if (uart_read_bytes(EX_UART_NUM, &ch, 1, pdMS_TO_TICKS(20)) > 0) {
            uart_write_bytes(EX_UART_NUM, ">> SEÑAL ENVIADA\r\n", 18);
            xSemaphoreGive(xBinarySemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main() {
    uart_config_t uart_cfg = { .baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .source_clk = UART_SCLK_APB };
    uart_param_config(EX_UART_NUM, &uart_cfg);
    uart_set_pin(EX_UART_NUM, TX_PIN, RX_PIN, -1, -1);
    uart_driver_install(EX_UART_NUM, 1024 * 2, 0, 0, NULL, 0);
    
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    xBinarySemaphore = xSemaphoreCreateBinary();
    xMutex = xSemaphoreCreateMutex();

    // Creación de las 3 tareas con prioridades
    xTaskCreate(vTaskUART, "UART", 4096, NULL, 5, NULL);
    xTaskCreate(vTaskProcessor, "Procesador", 2048, NULL, 4, NULL);
    xTaskCreate(vTaskReport, "Monitor", 2048, NULL, 3, NULL);
}

#ifdef __cplusplus
}
#endif