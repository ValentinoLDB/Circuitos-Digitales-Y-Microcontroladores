#ifndef APP_H
#define APP_H

#include <stdint.h>

/**
 *  Inicializa el estado interno de la aplicacion.
 */
void app_init(void);

/**
 *  Ejecuta la tarea de telemetria.
 */
void app_telemetry_task(void);

/**
 *  Procesa los comandos recibidos por UART.
 */
void app_command_task(void);

#endif
