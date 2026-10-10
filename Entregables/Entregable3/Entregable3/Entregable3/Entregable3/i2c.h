#ifndef I2C_H
#define I2C_H

#include <stdint.h>

/**
 *  Inicializa el modulo I2C en modo maestro.
 */
void i2c_init(void);

/**
 *  Genera una condicion de inicio I2C.
 */
void i2c_start(void);

/**
 *  Genera una condicion de parada I2C.
 */
void i2c_stop(void);

/**
 *  Escribe un byte por I2C.
 */
void i2c_write(uint8_t data);

/**
 *  Lee un byte con ACK.
 */
uint8_t i2c_read_ack(void);

/**
 *  Lee un byte con NACK.
 */
uint8_t i2c_read_nack(void);

#endif
