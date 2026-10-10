#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// ==========================================
// VARIABLES GLOBALES Y BANDERAS
// ==========================================

volatile uint8_t flag_un_segundo = 0;
volatile uint8_t segundos_transcurridos = 0;
uint8_t intervalo_muestreo = 5;

volatile char rx_buffer[32];
volatile uint8_t rx_index = 0;
volatile uint8_t flag_comando_recibido = 0;

// --- BUFFER CIRCULAR DE TRANSMISIÓN (TX) ---
#define TX_BUFFER_SIZE 128
volatile char tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;

uint8_t horas = 0, minutos = 0, segundos = 0;
uint8_t temperatura = 0, humedad = 0;

uint8_t contador_alerta_temp = 0;
uint8_t contador_alerta_hum = 0;

// ==========================================
// DRIVERS Y UTILIDADES BÁSICAS I2C / BCD
// ==========================================

void i2c_init(void) { TWBR = 72; TWSR = 0x00; TWCR = (1 << TWEN); }

void i2c_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void) { TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); }

void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

uint8_t i2c_read_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

uint8_t bcd_a_decimal(uint8_t val) { return ((val / 16) * 10) + (val % 16); }
uint8_t decimal_a_bcd(uint8_t val) { return ((val / 10) * 16) + (val % 10); }

// ==========================================
// FUNCIONES DE RELOJ (DS3232)
// ==========================================

void leer_hora_ds3232(void) {
	i2c_start();
	i2c_write(0xD0);
	i2c_write(0x00);
	i2c_start();
	i2c_write(0xD1);
	segundos = bcd_a_decimal(i2c_read_ack());
	minutos = bcd_a_decimal(i2c_read_ack());
	horas = bcd_a_decimal(i2c_read_nack());
	i2c_stop();
}

void escribir_hora_ds3232(uint8_t h, uint8_t m, uint8_t s) {
	i2c_start();
	i2c_write(0xD0);
	i2c_write(0x00);
	i2c_write(decimal_a_bcd(s));
	i2c_write(decimal_a_bcd(m));
	i2c_write(decimal_a_bcd(h));
	i2c_stop();
}

// ==========================================
// DRIVER SENSOR DHT11
// ==========================================

uint8_t dht11_read(uint8_t *temp, uint8_t *hum) {
	uint8_t bits[5] = {0,0,0,0,0};
	uint16_t timeout;
	
	DDRC |= (1 << PC0);
	PORTC &= ~(1 << PC0);
	_delay_ms(20);
	PORTC |= (1 << PC0);
	DDRC &= ~(1 << PC0);
	
	timeout = 0; while(PINC & (1<<PC0)) { if(++timeout > 10000) return 0; }
	timeout = 0; while(!(PINC & (1<<PC0))) { if(++timeout > 10000) return 0; }
	timeout = 0; while(PINC & (1<<PC0)) { if(++timeout > 10000) return 0; }
	
	for (uint8_t j = 0; j < 5; j++) {
		for (uint8_t i = 0; i < 8; i++) {
			timeout = 0; while(!(PINC & (1<<PC0))) { if(++timeout > 10000) return 0; }
			_delay_us(40);
			if (PINC & (1<<PC0)) {
				bits[j] |= (1 << (7 - i));
				timeout = 0; while(PINC & (1<<PC0)) { if(++timeout > 10000) return 0; }
			}
		}
	}
	
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		*hum = bits[0];
		*temp = bits[2];
		return 1;
	}
	return 0;
}

// ==========================================
// UTILIDADES BARE-METAL PARA TEXTO
// ==========================================

void SerialPort_Send_Char(char data) {
	uint8_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;
	while (next_head == tx_tail);
	tx_buffer[tx_head] = data;
	tx_head = next_head;
	UCSR0B |= (1 << UDRIE0);
}

void SerialPort_Send_String(const char * msg){
	uint8_t i = 0;
	while(msg[i]){
		SerialPort_Send_Char(msg[i]);
		i++;
	}
}

void SerialPort_Send_Num2(uint8_t num) {
	char d1 = (num / 10) + '0';
	char d2 = (num % 10) + '0';
	SerialPort_Send_Char(d1);
	SerialPort_Send_Char(d2);
}

uint8_t get_length(volatile char* str) {
	uint8_t len = 0;
	while(str[len]) len++;
	return len;
}

uint8_t match_prefix(volatile char* str, const char* prefix) {
	uint8_t i = 0;
	while(prefix[i] != '\0') {
		if(str[i] != prefix[i]) return 0;
		i++;
	}
	return 1;
}

// ==========================================
// INICIALIZACIÓN DE HARDWARE
// ==========================================

void init_UART(void) {
	UBRR0H = (103 >> 8); UBRR0L = 103;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void init_Timer1_1Seg(void) {
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	OCR1A = 62499;
	TIMSK1 |= (1 << OCIE1A);
}

// ==========================================
// INTERRUPCIONES
// ==========================================

ISR(TIMER1_COMPA_vect) {
	flag_un_segundo = 1;
}

ISR(USART_RX_vect) {
	char data = UDR0;
	
	if (data == '\b' || data == 127) {
		if (rx_index > 0) {
			rx_index--;
			SerialPort_Send_Char('\b');
			SerialPort_Send_Char(' ');
			SerialPort_Send_Char('\b');
		}
	}
	else if (data == '\r' || data == '\n') {
		SerialPort_Send_String("\r\n");
		rx_buffer[rx_index] = '\0';
		flag_comando_recibido = 1;
		rx_index = 0;
	}
	else {
		if (rx_index < 31) {
			rx_buffer[rx_index++] = data;
			SerialPort_Send_Char(data);
		}
	}
}

ISR(USART_UDRE_vect) {
	if (tx_head != tx_tail) {
		UDR0 = tx_buffer[tx_tail];
		tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
		} else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}

// ==========================================
// PROGRAMA PRINCIPAL
// ==========================================

int main(void) {
	
	init_UART();
	init_Timer1_1Seg();
	i2c_init();
	sei();
	
	SerialPort_Send_String("Iniciando Monitor de Invernadero...\r\n");
	
	while (1) {
		
		// --- TAREA A: Temporización y Telemetría ---
		if (flag_un_segundo) {
			flag_un_segundo = 0;
			segundos_transcurridos++;
			
			if (segundos_transcurridos >= intervalo_muestreo) {
				segundos_transcurridos = 0;
				
				leer_hora_ds3232();
				
				if (dht11_read(&temperatura, &humedad)) {
					uint8_t es_dia = (horas >= 7 && horas <= 18);
					uint8_t temp_ok = 0, hum_ok = 0;
					const char* periodo_str = "";
					
					if (es_dia) {
						periodo_str = "diurno";
						temp_ok = (temperatura >= 20 && temperatura <= 30);
						hum_ok = (humedad >= 50 && humedad <= 70);
						} else {
						periodo_str = "nocturno";
						temp_ok = (temperatura >= 15 && temperatura <= 22);
						hum_ok = (humedad >= 60 && humedad <= 80);
					}

					// --- 1. SALTO DE LÍNEA PREVENTIVO ---
					// Si el usuario estaba escribiendo, bajamos un renglón
					// para que la alerta aparezca abajo sin mezclarse.
					if (rx_index > 0) {
						SerialPort_Send_String("\r\n");
					}
					
					// --- 2. IMPRESIÓN TELEMETRÍA ---
					SerialPort_Send_String("[");
					SerialPort_Send_Num2(horas); SerialPort_Send_String(":");
					SerialPort_Send_Num2(minutos); SerialPort_Send_String(":");
					SerialPort_Send_Num2(segundos);
					SerialPort_Send_String("] T: ");
					SerialPort_Send_Num2(temperatura);
					SerialPort_Send_String("C H: ");
					SerialPort_Send_Num2(humedad);
					SerialPort_Send_String("% | Estado: ");
					
					if (temp_ok && hum_ok) {
						SerialPort_Send_String("NORMAL\r\n");
						} else {
						SerialPort_Send_String("ALERTA\r\n");
					}
					
					// IMPRESIÓN ALERTAS INDEPENDIENTES
					if (!temp_ok) {
						contador_alerta_temp++;
						if (contador_alerta_temp % 2 == 0) {
							SerialPort_Send_String("[ALERTA] [");
							SerialPort_Send_Num2(horas); SerialPort_Send_String(":");
							SerialPort_Send_Num2(minutos); SerialPort_Send_String(":");
							SerialPort_Send_Num2(segundos);
							SerialPort_Send_String("] Temperatura fuera de rango ");
							SerialPort_Send_String(periodo_str);
							SerialPort_Send_String("! Valor: ");
							SerialPort_Send_Num2(temperatura);
							SerialPort_Send_String("C\r\n");
						}
						} else {
						contador_alerta_temp = 0;
					}
					
					if (!hum_ok) {
						contador_alerta_hum++;
						if (contador_alerta_hum % 2 == 0) {
							SerialPort_Send_String("[ALERTA] [");
							SerialPort_Send_Num2(horas); SerialPort_Send_String(":");
							SerialPort_Send_Num2(minutos); SerialPort_Send_String(":");
							SerialPort_Send_Num2(segundos);
							SerialPort_Send_String("] Humedad fuera de rango ");
							SerialPort_Send_String(periodo_str);
							SerialPort_Send_String("! Valor: ");
							SerialPort_Send_Num2(humedad);
							SerialPort_Send_String("%\r\n");
						}
						} else {
						contador_alerta_hum = 0;
					}

					// --- 3. DEVOLVER EL PROMPT AL USUARIO EN EL NUEVO RENGLÓN ---
					if (rx_index > 0) {
						SerialPort_Send_String("> ");
						for (uint8_t i = 0; i < rx_index; i++) {
							SerialPort_Send_Char(rx_buffer[i]);
						}
					}
					
					} else {
					
					// Salto de línea preventivo en error
					if (rx_index > 0) {
						SerialPort_Send_String("\r\n");
					}
					
					SerialPort_Send_String("Error: No se detecta el sensor DHT11.\r\n");
					
					if (rx_index > 0) {
						SerialPort_Send_String("> ");
						for (uint8_t i = 0; i < rx_index; i++) {
							SerialPort_Send_Char(rx_buffer[i]);
						}
					}
				}
			}
		}
		
		// --- TAREA B: Comandos por Consola ---
		if (flag_comando_recibido) {
			flag_comando_recibido = 0;
			uint8_t len = get_length(rx_buffer);
			
			if (match_prefix(rx_buffer, "SET TM=")) {
				if (len == 8 || len == 9) {
					uint8_t nuevo_t = 0;
					if (len == 8) {
						nuevo_t = rx_buffer[7] - '0';
						} else {
						nuevo_t = (rx_buffer[7] - '0') * 10 + (rx_buffer[8] - '0');
					}
					
					if (nuevo_t >= 2 && nuevo_t <= 60) {
						intervalo_muestreo = nuevo_t;
						SerialPort_Send_String("Intervalo actualizado correctamente.\r\n");
						} else {
						SerialPort_Send_String("Error: El intervalo debe ser entre 2 y 60 seg.\r\n");
					}
					} else {
					SerialPort_Send_String("Error: Formato invalido para SET TM.\r\n");
				}
			}
			else if (match_prefix(rx_buffer, "SET TIME=")) {
				if (len == 17 && rx_buffer[11] == ':' && rx_buffer[14] == ':') {
					uint8_t h = (rx_buffer[9] - '0') * 10 + (rx_buffer[10] - '0');
					uint8_t m = (rx_buffer[12] - '0') * 10 + (rx_buffer[13] - '0');
					uint8_t s = (rx_buffer[15] - '0') * 10 + (rx_buffer[16] - '0');
					
					if (h <= 23 && m <= 59 && s <= 59) {
						escribir_hora_ds3232(h, m, s);
						SerialPort_Send_String("Hora del RTC actualizada.\r\n");
						} else {
						SerialPort_Send_String("Error: Hora invalida. Maximo 23:59:59.\r\n");
					}
					} else {
					SerialPort_Send_String("Error: Use formato exacto SET TIME=HH:MM:SS\r\n");
				}
			}
			else if (len > 0) {
				SerialPort_Send_String("Comando desconocido.\r\n");
			}
		}
	}
	return 0;
}