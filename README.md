# Tarea 5 – Sistemas Embebidos con ESP32 y FreeRTOS

Este repositorio contiene el desarrollo de la Tarea #5 de la asignatura Sistemas Embebidos. El proyecto se enfoca en la implementación de comunicación serial bidireccional y sistemas multitarea en tiempo real utilizando ESP32, ESP-IDF y FreeRTOS.
Funcionamiento: https://youtu.be/b-aOoY6wn2A

## Integrantes
- Daniela Basilio
- Shirley Esquibel
- Melisa Castro
- Alexander Gallegos

## Descripción del proyecto
El objetivo del proyecto es analizar la gestión de recursos en un microcontrolador ESP32, partiendo de un enfoque basado en polling y evolucionando hacia un sistema multitarea con FreeRTOS, incorporando comunicación entre tareas y mecanismos de sincronización.

El sistema funciona como un intérprete de comandos a través de UART2, permitiendo el control de hardware (LED) y la supervisión del estado del sistema en tiempo real.

## Configuración de hardware
- Microcontrolador: ESP32
- LED indicador: GPIO 2 (LED onboard)
- Interfaz de comunicación: UART2
  - TX: GPIO 17
  - RX: GPIO 16
  - Baud rate: 115200 bps
  - Configuración: 8 bits de datos, sin paridad, 1 bit de parada

## Estructura del proyecto

### Ejercicio 1: Comunicación serial (Polling)
Implementación de un sistema basado en un bucle infinito que consulta continuamente la UART.
- Lectura no bloqueante
- Limpieza del buffer
- Eco de caracteres recibidos

Comandos disponibles:
- `led on`  : Enciende el LED
- `led off` : Apaga el LED
- `status`  : Muestra el estado actual del sistema
- `info`    : Muestra el contador de comandos procesados
- `reset`   : Reinicia el contador de comandos

### Ejercicio 2: Multitarea con FreeRTOS
Migración a un sistema operativo de tiempo real con planificación por prioridades.
- Task UART (Prioridad 5): Gestión de la interfaz de usuario
- Task LED (Prioridad 4): Control del parpadeo del LED
- Task Monitor (Prioridad 3): Reporte periódico del estado del sistema cada 5 segundos

### Ejercicio 3: Uso de colas (Queues)
Implementación de colas para desacoplar tareas y permitir el paso seguro de mensajes.
- La tarea UART actúa como productora de comandos
- La tarea LED actúa como consumidora
- Se garantiza ejecución segura en entornos concurrentes

### Ejercicio 4: Sincronización (Semáforos y Mutex)
Protección de recursos compartidos mediante mecanismos de sincronización.
- Semáforo binario: Señalización desde la UART para activar tareas de procesamiento
- Mutex: Protección de variables globales para evitar condiciones de carrera

## Requisitos de software
- Visual Studio Code
- ESP-IDF o PlatformIO
- Simulador Wokwi (opcional para pruebas sin hardware)
