#include "dht11.h"
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

/* Driver de lectura para el sensor DHT11. */

// Lee una muestra completa del DHT11 y valida el checksum.
uint8_t dht11_read(uint8_t *temp, uint8_t *hum) {
	uint8_t bits[5] = {0, 0, 0, 0, 0};
	uint16_t timeout;

	// Enviar señal de inicio
	DDRC |= (1 << PC0);
	PORTC &= ~(1 << PC0);
	_delay_ms(20);
	PORTC |= (1 << PC0);
	DDRC &= ~(1 << PC0);

	// Esperar respuesta
	timeout = 0;
	while (PINC & (1 << PC0)) {
		if (++timeout > 10000)
			return 0;
	}
	timeout = 0;
	while (!(PINC & (1 << PC0))) {
		if (++timeout > 10000)
			return 0;
	}
	timeout = 0;
	while (PINC & (1 << PC0)) {
		if (++timeout > 10000)
			return 0;
	}

	// Leer 5 bytes (40 bits)
	for (uint8_t j = 0; j < 5; j++) {
		for (uint8_t i = 0; i < 8; i++) {
			timeout = 0;
			while (!(PINC & (1 << PC0))) {
				if (++timeout > 10000)
					return 0;
			}
			_delay_us(40);
			if (PINC & (1 << PC0)) {
				bits[j] |= (1 << (7 - i));
				timeout = 0;
				while (PINC & (1 << PC0)) {
					if (++timeout > 10000)
						return 0;
				}
			}
		}
	}

	// Validar checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		*hum = bits[0];
		*temp = bits[2];
		return 1;
	}
	return 0;
}
