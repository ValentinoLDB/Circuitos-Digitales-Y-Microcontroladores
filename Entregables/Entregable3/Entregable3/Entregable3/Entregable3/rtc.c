#include "rtc.h"
#include "i2c.h"

/* Acceso de alto nivel al RTC DS3232. */

// Convierte un valor BCD a decimal
static uint8_t bcd_a_decimal(uint8_t val) {
	return ((val / 16) * 10) + (val % 16);
}

// Convierte un valor decimal a BCD
static uint8_t decimal_a_bcd(uint8_t val) {
	return ((val / 10) * 16) + (val % 10);
}

// Lee la hora actual del RTC en formato decimal.
void rtc_read_time(uint8_t *h, uint8_t *m, uint8_t *s) {
	i2c_start();
	i2c_write(0xD0);
	i2c_write(0x00);
	i2c_start();
	i2c_write(0xD1);
	*s = bcd_a_decimal(i2c_read_ack());
	*m = bcd_a_decimal(i2c_read_ack());
	*h = bcd_a_decimal(i2c_read_nack());
	i2c_stop();
}

// Escribe la hora actual del RTC a partir de valores decimales.
void rtc_write_time(uint8_t h, uint8_t m, uint8_t s) {
	i2c_start();
	i2c_write(0xD0);
	i2c_write(0x00);
	i2c_write(decimal_a_bcd(s));
	i2c_write(decimal_a_bcd(m));
	i2c_write(decimal_a_bcd(h));
	i2c_stop();
}
