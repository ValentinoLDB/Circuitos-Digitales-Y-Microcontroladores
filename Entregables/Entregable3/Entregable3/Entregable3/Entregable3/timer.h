#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 *  Inicializa Timer1 para generar una interrupcion por segundo.
 */
void timer_init(void);

/**
 *  Consume la marca de un segundo si esta disponible.
 */
uint8_t timer_tick(void);

/**
 *  Devuelve los segundos acumulados desde el ultimo reset.
 */
uint8_t timer_get_elapsed_seconds(void);

/**
 *  Reinicia el contador de segundos acumulados.
 */
void timer_reset_elapsed(void);

#endif
