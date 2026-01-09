#include <stdio.h>
#include <string.h>
#include <ctype.h> // Necesario para limpiar la cadena
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define EX_UART_NUM UART_NUM_2
#define TX_PIN 17
#define RX_PIN 16
#define BUF_SIZE 1024
#define LED_PIN 2

#ifdef __cplusplus
extern "C" {
#endif

// Función para limpiar el comando (quita espacios, saltos de línea y basura)
void clean_buffer(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        } else if (dst > str && *(dst-1) != ' ') {
            *dst++ = ' '; // Mantenemos un solo espacio entre palabras (ej: "led on")
        }
        src++;
    }
    *dst = '\0';
    // Quitar espacio final si existe
    if (dst > str && *(dst-1) == ' ') *(dst-1) = '\0';
}

void app_main() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_pin(EX_UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    char line_buffer[128];
    int line_pos = 0;
    uint8_t ch;
    int cmd_count = 0;

    vTaskDelay(pdMS_TO_TICKS(500));
    uart_write_bytes(EX_UART_NUM, "\r\n--- CONSOLA REINICIADA: ESCRIBE AHORA ---\r\n", 45);

    while (1) {
        if (uart_read_bytes(EX_UART_NUM, &ch, 1, pdMS_TO_TICKS(20)) > 0) {
            
            if (ch == '\n' || ch == '\r') {
                if (line_pos > 0) {
                    line_buffer[line_pos] = '\0';
                    uart_write_bytes(EX_UART_NUM, "\r\n", 2); 

                    clean_buffer(line_buffer); // Limpiamos el comando antes de comparar

                    if (strcmp(line_buffer, "status") == 0) {
                        cmd_count++;
                        uart_write_bytes(EX_UART_NUM, ">> SISTEMA OK\r\n", 16);
                    } 
                    else if (strcmp(line_buffer, "led on") == 0) {
                        cmd_count++;
                        gpio_set_level(LED_PIN, 1);
                        uart_write_bytes(EX_UART_NUM, ">> LED ENCENDIDO\r\n", 19);
                    } 
                    else if (strcmp(line_buffer, "led off") == 0) {
                        cmd_count++;
                        gpio_set_level(LED_PIN, 0);
                        uart_write_bytes(EX_UART_NUM, ">> LED APAGADO\r\n", 17);
                    } 
                    else if (strcmp(line_buffer, "info") == 0) {
                        cmd_count++;
                        char info_str[64];
                        int n = sprintf(info_str, ">> UART2 | COMANDOS: %d\r\n", cmd_count);
                        uart_write_bytes(EX_UART_NUM, info_str, n);
                    } 
                    else if (strcmp(line_buffer, "reset") == 0) {
                        cmd_count = 0;
                        uart_write_bytes(EX_UART_NUM, ">> CONTADOR EN CERO\r\n", 22);
                    } 
                    else {
                        uart_write_bytes(EX_UART_NUM, ">> ERROR: INTENTA DE NUEVO\r\n", 29);
                    }
                    line_pos = 0; 
                }
            } 
            else if (line_pos < sizeof(line_buffer) - 1) {
                uart_write_bytes(EX_UART_NUM, (const char*)&ch, 1); 
                line_buffer[line_pos++] = ch;
            }
        }
    }
}

#ifdef __cplusplus
}
#endif