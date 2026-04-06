#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	// 1. CONFIGURACIÓN DE HARDWARE
	DDRD = 0xFF;    // Puerto D todo como salida (Para los 8 LEDs)
	PORTD = 0x00;   // Arrancamos con todos los LEDs apagados

	DDRC &= ~(1 << DDC0);  // Pin PC0 como entrada (Pulsador 1)
	PORTC |= (1 << PORTC0); // Habilitar resistencia Pull-up en PC0

	// 2. VARIABLES DE LA MÁQUINA DE ESTADOS
	int secuencia_actual = 0; // 0 = Secuencia 'a' (Sube), 1 = Secuencia 'b' (Rebota)
	int paso = 0;             // Indica qué bit del LED hay que prender (0 a 7)
	int direccion = 1;        // 1 = Hacia la izquierda, -1 = Hacia la derecha (Para el rebote)
	
	int estado_boton_anterior = 1; // Asumimos que arranca sin apretar (1 lógico por el pull-up)

	// 3. EL SUPER LOOP
	while (1) {
		
		// --- TAREA A: ACTUALIZAR LAS LUCES ---
		PORTD = (1 << paso); // Prende el LED correspondiente al "paso" actual

		if (secuencia_actual == 0) {
			// Lógica de Secuencia 'a' (LSB hacia MSB y repite)
			paso++;
			if (paso > 7) {
				paso = 0; // Si se pasó del último, vuelve al primero
			}
		}
		else {
			// Lógica de Secuencia 'b' (MSB, baja y rebota en los extremos)
			paso += direccion;
			if (paso >= 7) {
				direccion = -1; // Si toca el techo (MSB), cambia la dirección para bajar
			}
			if (paso <= 0) {
				direccion = 1;  // Si toca el piso (LSB), cambia la dirección para subir
			}
		}

		// --- TAREA B: LA ESPERA "NO BLOQUEANTE" (El truco del profe) ---
		// Queremos esperar 100ms, pero lo hacemos en 10 cuotas de 10ms.
		int cambiar_secuencia = 0; // Bandera para saber si apretaron el botón

		for (int i = 0; i < 10; i++) {
			_delay_ms(10);
			
			// Leemos el botón en C0 (0 significa que está apretado por ir a tierra)
			int estado_boton_actual = (PINC & (1 << PINC0)) ? 1 : 0;

			// Detección de "Flanco Descendente": Solo actuamos si ANTES estaba suelto y AHORA está apretado
			if (estado_boton_anterior == 1 && estado_boton_actual == 0) {
				cambiar_secuencia = 1;
			}
			estado_boton_anterior = estado_boton_actual; // Guardamos el estado para la próxima vuelta
		}

		// --- TAREA C: GESTIONAR EL CAMBIO ---
		if (cambiar_secuencia == 1) {
			if (secuencia_actual == 0) {
				secuencia_actual = 1; // Pasamos a secuencia 'b'
				paso = 7;             // Arrancamos desde el MSB como pide el TP
				direccion = -1;       // Configurado para bajar
				} else {
				secuencia_actual = 0; // Volvemos a secuencia 'a'
				paso = 0;             // Arrancamos desde LSB
			}
		}
		
	} // Fin del while(1)
}