#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Temporizador del sistema basado en Timer1. */

volatile uint8_t flag_un_segundo = 0;
volatile uint8_t segundos_transcurridos = 0;

// Configura Timer1 para generar una interrupcion cada 1 segundo.
void timer_init(void) {
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	OCR1A = 62499;
	TIMSK1 |= (1 << OCIE1A);
}

// Consume la marca de un segundo si esta pendiente.
uint8_t timer_tick(void) {
	if (flag_un_segundo) {
		flag_un_segundo = 0;
		segundos_transcurridos++;
		return 1;
	}
	return 0;
}

// Devuelve los segundos acumulados desde el ultimo reset.
uint8_t timer_get_elapsed_seconds(void) {
	return segundos_transcurridos;
}

// Reinicia el acumulador de tiempo.
void timer_reset_elapsed(void) {
	segundos_transcurridos = 0;
}

// ISR del comparador A: marca el paso de 1 segundo.
ISR(TIMER1_COMPA_vect) {
	flag_un_segundo = 1;
}
