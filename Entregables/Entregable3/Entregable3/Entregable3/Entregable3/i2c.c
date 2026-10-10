#include "i2c.h"
#include <avr/io.h>

/* Implementacion minima del bus I2C/TWI en modo maestro. */

// Inicializa el hardware TWI.
void i2c_init(void) {
	TWBR = 72;
	TWSR = 0x00;
	TWCR = (1 << TWEN);
}

// Genera una condicion de inicio I2C.
void i2c_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// Genera una condicion de parada I2C.
void i2c_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Escribe un byte por el bus I2C.
void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// Lee un byte con ACK.
uint8_t i2c_read_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// Lee un byte con NACK.
uint8_t i2c_read_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}
