#include "uart.h"
#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#define BUFFER_SIZE 32

static volatile char rx_buffer[BUFFER_SIZE];
static volatile uint8_t rx_index = 0;
static volatile uint8_t flag_comando_listo = 0;

ISR(USART_RX_vect) {
    char c = UDR0;

    if (flag_comando_listo == 0) {
        if (c == '\n' || c == '\r') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';
                flag_comando_listo = 1;
            }
        } else if (rx_index < BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = c;
        }
    }
}

void UART_Init(void) {
    UBRR0 = 103;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendString(const char *str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

void UART_ParseCommand(void) {
    if (flag_comando_listo) {
        char *ptr = strstr((char*)rx_buffer, "SET_COLOR");
        if (ptr != NULL) {
            ptr = strchr(ptr, '=');
            if (ptr != NULL) {
                ptr++;
                
                int r = 0, g = 0, b = 0;
                uint8_t formato_ok = 1;
                
                while(*ptr == ' ') ptr++;
                if (!(*ptr >= '0' && *ptr <= '9')) formato_ok = 0;
                while(*ptr >= '0' && *ptr <= '9') { r = r * 10 + (*ptr - '0'); ptr++; }
                
                if (formato_ok && *ptr == ',') {
                    ptr++;
                    while(*ptr == ' ') ptr++;
                    if (!(*ptr >= '0' && *ptr <= '9')) formato_ok = 0;
                    while(*ptr >= '0' && *ptr <= '9') { g = g * 10 + (*ptr - '0'); ptr++; }
                    
                    if (formato_ok && *ptr == ',') {
                        ptr++;
                        while(*ptr == ' ') ptr++;
                        if (!(*ptr >= '0' && *ptr <= '9')) formato_ok = 0;
                        while(*ptr >= '0' && *ptr <= '9') { b = b * 10 + (*ptr - '0'); ptr++; }
                        
                        if (formato_ok && (r > 255 || g > 255 || b > 255)) {
                            formato_ok = 0;
                        }
                        
                        if (formato_ok) {
                            PWM_SetColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
                        } else {
                            UART_SendString("FORMATO ERRONEO, FORMATO ADECUADO:SET_COLOR=X,X,X   0<=X<=255\r\n");
                        }
                    } else {
                        UART_SendString("FORMATO ERRONEO, FORMATO ADECUADO:SET_COLOR=X,X,X   0<=X<=255\r\n");
                    }
                } else {
                    UART_SendString("FORMATO ERRONEO, FORMATO ADECUADO:SET_COLOR=X,X,X   0<=X<=255\r\n");
                }
            } else {
                UART_SendString("FORMATO ERRONEO, FORMATO ADECUADO:SET_COLOR=X,X,X   0<=X<=255\r\n");
            }
        } else {
            UART_SendString("Comando desconocido.\r\n");
            UART_SendString("FORMATO ADECUADO:SET_COLOR=X,X,X   0<=X<=255\r\n");
        }
        
        rx_index = 0;
        flag_comando_listo = 0;
    }
}
