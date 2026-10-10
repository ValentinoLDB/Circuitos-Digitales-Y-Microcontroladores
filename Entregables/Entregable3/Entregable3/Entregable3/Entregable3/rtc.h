#ifndef RTC_H
#define RTC_H

#include <stdint.h>

/**
 *  Lee la hora actual del RTC DS3232.
 */
void rtc_read_time(uint8_t *h, uint8_t *m, uint8_t *s);

/**
 *  Escribe la hora actual en el RTC DS3232.
 */
void rtc_write_time(uint8_t h, uint8_t m, uint8_t s);

#endif
