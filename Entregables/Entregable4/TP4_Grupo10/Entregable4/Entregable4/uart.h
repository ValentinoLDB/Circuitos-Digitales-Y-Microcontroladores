#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

void UART_Init(void);
void UART_SendString(const char *str);
void UART_ParseCommand(void);

#endif /* _UART_H_ */
