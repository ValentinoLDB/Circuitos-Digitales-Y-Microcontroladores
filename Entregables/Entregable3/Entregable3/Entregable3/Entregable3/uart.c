#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Driver UART con buffers circulares e interrupciones. */

#define TX_BUFFER_SIZE 128
#define RX_BUFFER_SIZE 32

volatile char tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t flag_comando_recibido = 0;

// Configura la USART en 9600 baudios, 8N1.
void uart_init(void) {
	UBRR0L = 103;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Encola un caracter para transmision.
void uart_send_char(char data) {
	uint8_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
	while (next_head == tx_tail) {
		if (!(SREG & (1 << SREG_I))) {
			return;
		}
	}
	tx_buffer[tx_head] = data;
	tx_head = next_head;
	UCSR0B |= (1 << UDRIE0);
}

// Envia una cadena terminada en nulo.
void uart_send_string(const char *msg) {
	uint8_t i = 0;
	while (msg[i]) {
		uart_send_char(msg[i]);
		i++;
	}
}

// Envia un numero de dos digitos.
void uart_send_num2(uint8_t num) {
	char d1 = (num / 10) + '0';
	char d2 = (num % 10) + '0';
	uart_send_char(d1);
	uart_send_char(d2);
}

// Indica si ya se recibio un comando completo.
uint8_t uart_command_ready(void) {
	return flag_comando_recibido;
}

// Devuelve el buffer interno del comando recibido.
volatile char* uart_get_command(void) {
	return rx_buffer;
}

// Limpia la bandera de comando listo.
void uart_clear_command(void) {
	flag_comando_recibido = 0;
}

// Devuelve el indice actual de escritura del buffer RX.
uint8_t uart_get_rx_index(void) {
	return rx_index;
}

// ISR de recepcion: arma el comando y maneja backspace/enter.
ISR(USART_RX_vect) {
	char data = UDR0;

	if (data == '\b' || data == 127) {
		if (rx_index > 0) {
			rx_index--;
			uart_send_char('\b');
			uart_send_char(' ');
			uart_send_char('\b');
		}
	}
	else if (data == '\r' || data == '\n') {
		uart_send_string("\r\n");
		rx_buffer[rx_index] = '\0';
		flag_comando_recibido = 1;
		rx_index = 0;
	}
	else {
		if (rx_index < RX_BUFFER_SIZE - 1) {
			rx_buffer[rx_index++] = data;
			uart_send_char(data);
		}
	}
}

// ISR de buffer vacio: continua la transmision pendiente.
ISR(USART_UDRE_vect) {
	if (tx_head != tx_tail) {
		UDR0 = tx_buffer[tx_tail];
		tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
	}
	else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}
