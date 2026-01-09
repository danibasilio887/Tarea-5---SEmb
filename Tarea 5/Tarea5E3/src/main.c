#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" // Librería obligatoria para el Ejercicio 3
#include "driver/uart.h"
#include "driver/gpio.h"

#define EX_UART_NUM UART_NUM_0
#define TX_PIN 1
#define RX_PIN 3
#define LED_PIN 2

// Definición de la Cola
QueueHandle_t xQueueComandos;

#ifdef __cplusplus
extern "C" {
#endif

// --- TAREA 1: RECEPTOR DE COLA (Control de LED) ---
void vTaskLEDLogic(void *pvParameters) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    int current_delay = 500;
    int state = 0;

    while (1) {
        int nuevo_delay;
        // Revisa si hay un mensaje en la cola (sin bloquearse si no hay nada)
        if (xQueueReceive(xQueueComandos, &nuevo_delay, 0) == pdPASS) {
            current_delay = nuevo_delay;
        }

        if (current_delay > 0) {
            state = !state;
            gpio_set_level(LED_PIN, state);
            vTaskDelay(pdMS_TO_TICKS(current_delay));
        } else {
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// --- TAREA 2: EMISOR DE COLA (Interfaz UART) ---
void vTaskUART(void *pvParameters) {
    uint8_t ch;
    char line[32];
    int pos = 0;
    int val_to_send;

    uart_write_bytes(EX_UART_NUM, "\r\n--- EJERCICIO 3: COMUNICACION POR COLAS ---\r\n", 47);

    while (1) {
        if (uart_read_bytes(EX_UART_NUM, &ch, 1, pdMS_TO_TICKS(20)) > 0) {
            if (ch == '\n' || ch == '\r') {
                line[pos] = '\0';
                uart_write_bytes(EX_UART_NUM, "\r\n", 2);

                if (strcmp(line, "fast") == 0) val_to_send = 100;
                else if (strcmp(line, "slow") == 0) val_to_send = 1000;
                else if (strcmp(line, "stop") == 0) val_to_send = 0;
                
                // ENVIAR EL DATO A LA COLA
                xQueueSend(xQueueComandos, &val_to_send, portMAX_DELAY);
                uart_write_bytes(EX_UART_NUM, ">> Comando enviado a la cola\r\n", 31);
                pos = 0;
            } else if (pos < 31) {
                uart_write_bytes(EX_UART_NUM, (const char*)&ch, 1);
                line[pos++] = ch;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main() {
    // Configuración UART2 (Igual que Ejercicios anteriores)
    uart_config_t uart_cfg = { .baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .source_clk = UART_SCLK_APB };
    uart_param_config(EX_UART_NUM, &uart_cfg);
    uart_set_pin(EX_UART_NUM, TX_PIN, RX_PIN, -1, -1);
    uart_driver_install(EX_UART_NUM, 1024 * 2, 0, 0, NULL, 0);

    // 1. CREAR LA COLA (Capacidad para 10 números enteros)
    xQueueComandos = xQueueCreate(10, sizeof(int));

    if (xQueueComandos != NULL) {
        xTaskCreate(vTaskLEDLogic, "Tarea_LED", 2048, NULL, 5, NULL);
        xTaskCreate(vTaskUART, "Tarea_UART", 4096, NULL, 5, NULL);
    }
}

#ifdef __cplusplus
}
#endif