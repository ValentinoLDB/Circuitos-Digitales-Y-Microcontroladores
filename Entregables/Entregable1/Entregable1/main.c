#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// FUNCIONES DEL NEOPIXEL (WS2812B)

void ws2812_mandar_byte(uint8_t dato){
	uint8_t mascara = 0x80;
	for(uint8_t i = 0; i < 8; i++){
		PORTB |= (1 << PORTB0);         // pongo el puerto en HIGH
		
		if(dato & mascara){             // aca analizo si el bit es un 1, en caso de serlo realizo la cantidad de nops necesarios para lograr un 1 en tiempo
			__asm__ __volatile__(
			"nop\n\t""nop\n\t""nop\n\t"
			"nop\n\t""nop\n\t""nop\n\t"
			"nop\n\t""nop\n\t"
			);  // 8 nops                            // tengo 2 nops de la asignacion port, 2-3 ciclos del if y agrego 8 nops para llegar a 13 ciclos aprox
		}
		PORTB &= ~(1 << PORTB0);        // pongo el puerto en low
		// agrego 2 nops para el caso que el bit sea 1
		__asm__ __volatile__(
		"nop\n\t""nop\n\t");
		
		if(!(dato & mascara)){          // la asignacion del puerto ya usa 2 ciclos y el if usara unos 2-3 ciclos todo en LOW
			__asm__ __volatile__(
			"nop\n\t""nop\n\t""nop\n\t"
			"nop\n\t""nop\n\t""nop\n\t"
			"nop\n\t""nop\n\t"
			); // 8 nops
		}
		dato = dato << 1;
	}// cuenta total de ciclos: caso bit = 1 tiene en HIGH 2 ciclos asignacion, 2-3 ciclos if y 8 nops = 12-13 ciclos en HIGH y en LOW tiene 2 asignacion, 2-3 if y 2 nops 6-7 ciclos en LOW
	// para el caso bit = 0 tiene en HIGH 2 ciclos asignacion 2-3 ciclos if que suma 4-5 ciclos en HIGH y en LOW tiene 2 asignacion, 2 nops, 2-3 del if y 8 nops  14-15 ciclos
	// asi ambos llegan casi a los 20 ciclos de un mensaje entero aproximado.
}

void ws2812_pintar_led(uint8_t rojo, uint8_t verde, uint8_t azul) {
	// El orden del WS2812B es GRB
	ws2812_mandar_byte(verde);
	ws2812_mandar_byte(rojo);
	ws2812_mandar_byte(azul);
}

void ws2812_latch(void) {
	_delay_us(55); // Resetea y muestra los colores
}

// PROGRAMA PRINCIPAL

int main(void) {
	// --- 1. CONFIGURACION DE HARDWARE ---
	DDRD = 0xFF;    // Puerto D salidas (8 LEDs comunes)
	PORTD = 0x00;   // Apagados por defecto
	
	DDRB |= (1 << DDB0); // PB0 salida (Datos Neopixel)
	PORTB &= ~(1 << PORTB0); // Arranca en 0V

	DDRC &= ~((1 << DDC0) | (1 << DDC1)); // PC0 y PC1 como entradas (Botones)
	PORTC |= (1 << PORTC0) | (1 << PORTC1); // Activar resistencias Pull-up internas

	// --- 2. VARIABLES DE ESTADO ---
	// Variables Puerto D
	int seq_D = 0;      // 0 = Sube (a), 1 = Rebota (b)
	int paso_D = 0;     // Que LED prender
	int dir_D = 1;      // Direccion del rebote
	
	// Variables Neopixel (Puerto B)
	int seq_B = 0;      // 0 = Pares/Impares (c), 1 = Chase verde (d)
	int paso_B = 7;     // Para la secuencia chase (arranca a la derecha)
	int estado_seq_c = 0; // 0 = Turno Rojo, 1 = Turno Azul

	// Contadores de tiempo (Ticks)
	int cont_10ms_D = 0;
	int cont_10ms_B = 0;

	// Estados anteriores de los botones (1 = sin apretar)
	int estado_C0_ant = 1;
	int estado_C1_ant = 1;

	// --- 3. EL SUPER LOOP ---
	while (1) {
		// EL RELOJ MAESTRO: Esperamos 10ms en cada vuelta del loop
		_delay_ms(10);
		cont_10ms_D++;
		cont_10ms_B++;

		// --- TAREA A: LECTURA DE BOTONES ---
		int estado_C0_act = (PINC & (1 << PINC0)) ? 1 : 0;
		int estado_C1_act = (PINC & (1 << PINC1)) ? 1 : 0;

		// Si se apreto el boton C0 (Cambio de secuencia D)
		if (estado_C0_ant == 1 && estado_C0_act == 0) {
			seq_D = (seq_D == 0) ? 1 : 0; // Alternar entre 0 y 1
			paso_D = (seq_D == 1) ? 7 : 0; // Si pasa a 'b', arranca arriba
			dir_D = -1;
		}
		estado_C0_ant = estado_C0_act;

		// Si se apreto el boton C1 (Cambio de secuencia Neopixel)
		if (estado_C1_ant == 1 && estado_C1_act == 0) {
			seq_B = (seq_B == 0) ? 1 : 0; // Alternar entre 0 y 1
			paso_B = 7; // Reset de la posicion del chase
		}
		estado_C1_ant = estado_C1_act;


		// --- TAREA B: ACTUALIZAR LEDs COMUNES (Cada 100ms) ---
		if (cont_10ms_D >= 10) {
			cont_10ms_D = 0; // Reiniciar contador
			
			PORTD = (1 << paso_D); // Dibujar en los LEDs

			if (seq_D == 0) {
				// Secuencia a: Sube de LSB a MSB
				paso_D++;
				if (paso_D > 7) paso_D = 0;
				} else {
				// Secuencia b: Baja y rebota
				paso_D += dir_D;
				if (paso_D >= 7) dir_D = -1;
				if (paso_D <= 0) dir_D = 1;
			}
		}

		// --- TAREA C: ACTUALIZAR NEOPIXELS (Cada 150ms) ---
		if (cont_10ms_B >= 15) {
			cont_10ms_B = 0; // Reiniciar contador
			
			if (seq_B == 0) {
				// Secuencia c: Pares rojos, LUEGO impares azules
				if (estado_seq_c == 0) {
					for(int i = 0; i < 8; i++) {
						if (i % 2 == 0) ws2812_pintar_led(255, 0, 0); // Par: Rojo
						else            ws2812_pintar_led(0, 0, 0);   // Impar: Apagado
					}
					estado_seq_c = 1; // Preparamos para que la proxima vez sea azul
					} else {
					for(int i = 0; i < 8; i++) {
						if (i % 2 != 0) ws2812_pintar_led(0, 0, 255); // Impar: Azul
						else            ws2812_pintar_led(0, 0, 0);   // Par: Apagado
					}
					estado_seq_c = 0; // Preparamos para que la proxima vez sea rojo
				}
				} else {
				// Secuencia d: Chase verde de derecha a izquierda
				for(int i = 0; i < 8; i++) {
					if (i == paso_B) ws2812_pintar_led(0, 255, 0); // Prender verde el actual
					else             ws2812_pintar_led(0, 0, 0);   // Apagar el resto
				}
				paso_B--; // Mover a la izquierda
				if (paso_B < 0) paso_B = 7; // Si llega al borde, vuelve a la derecha
			}
			
			ws2812_latch(); // Confirmar los colores a la tira
		}
		
	} // Fin del Super Loop
}