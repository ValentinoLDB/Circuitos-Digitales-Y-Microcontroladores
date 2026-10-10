#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

/**
 *  Lee una muestra del sensor DHT11.
 *
 * @return 1 si la lectura es valida, 0 si hubo error o timeout.
 */
uint8_t dht11_read(uint8_t *temp, uint8_t *hum);

#endif
