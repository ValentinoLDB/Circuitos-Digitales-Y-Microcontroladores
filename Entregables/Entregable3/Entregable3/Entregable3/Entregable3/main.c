#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "rtc.h"
#include "dht11.h"
#include "app.h"

/* Punto de entrada del sistema: inicializa perifericos y cede control al loop. */
int main(void) {
	// Inicializar módulos
	uart_init();
	timer_init();
	i2c_init();
	app_init();
	
	// Habilitar interrupciones globales
	sei();
	
	// Mensaje de arranque
	uart_send_string("Iniciando Monitor de Invernadero...\r\n");
	
	// Loop principal
	while (1) {
		// Tarea de telemetría
		app_telemetry_task();
		
		// Tarea de procesamiento de comandos
		app_command_task();
	}
	
	return 0;
}