#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 *  Inicializa la UART a 9600 baudios, 8N1.
 */
void uart_init(void);

/**
 *  Envia un caracter por UART usando buffer circular.
 */
void uart_send_char(char data);

/**
 *  Envia una cadena terminada en nulo.
 */
void uart_send_string(const char *msg);

/**
 *  Envia un numero de dos digitos en formato 00-99.
 */
void uart_send_num2(uint8_t num);

/**
 *  Indica si hay un comando completo recibido.
 */
uint8_t uart_command_ready(void);

/**
 *  Obtiene el buffer del ultimo comando recibido.
 */
volatile char* uart_get_command(void);

/**
 *  Devuelve la cantidad de caracteres recibidos.
 */
uint8_t uart_get_rx_index(void);

/**
 *  Limpia la bandera de comando listo.
 */
void uart_clear_command(void);

#endif
