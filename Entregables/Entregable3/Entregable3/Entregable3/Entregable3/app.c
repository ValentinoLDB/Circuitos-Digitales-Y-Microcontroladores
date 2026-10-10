#include "app.h"
#include "rtc.h"
#include "dht11.h"
#include "uart.h"
#include "timer.h"

/*
 * Capa de aplicacion del sistema.
 * Coordina la lectura de sensores, la evaluacion de umbrales y el procesamiento
 * de comandos sin depender de detalles de bajo nivel.
 */

// Configuracion de umbrales y ventanas de trabajo
#define INTERVALO_MUESTREO_DEFAULT 5
#define TEMP_DIA_MIN 20
#define TEMP_DIA_MAX 30
#define HUM_DIA_MIN 50
#define HUM_DIA_MAX 70
#define TEMP_NOCHE_MIN 15
#define TEMP_NOCHE_MAX 22
#define HUM_NOCHE_MIN 60
#define HUM_NOCHE_MAX 80
#define HORA_DIA_INICIO 7
#define HORA_DIA_FIN 18

// Variables de estado internas del modulo
static uint8_t intervalo_muestreo = INTERVALO_MUESTREO_DEFAULT;
static uint8_t contador_alerta_temp = 0;
static uint8_t contador_alerta_hum = 0;

// Calcula la longitud de una cadena terminada en nulo.
static uint8_t str_length(volatile char *str) {
	uint8_t len = 0;
	while (str[len])
		len++;
	return len;
}

// Verifica si la cadena comienza con un prefijo dado.
static uint8_t str_match_prefix(volatile char *str, const char *prefix) {
	uint8_t i = 0;
	while (prefix[i] != '\0') {
		if (str[i] != prefix[i])
			return 0;
		i++;
	}
	return 1;
}

// Emite una linea de telemetria con formato fijo para consola serial.
static void print_telemetry(uint8_t h, uint8_t m, uint8_t s, uint8_t temp,
			    uint8_t hum, uint8_t estado) {
	uart_send_string("[");
	uart_send_num2(h);
	uart_send_string(":");
	uart_send_num2(m);
	uart_send_string(":");
	uart_send_num2(s);
	uart_send_string("] T: ");
	uart_send_num2(temp);
	uart_send_string("C H: ");
	uart_send_num2(hum);
	uart_send_string("% | Estado: ");
	if (estado) {
		uart_send_string("NORMAL\r\n");
	} else {
		uart_send_string("ALERTA\r\n");
	}
}

// Emite una alerta formateada para temperatura o humedad fuera de rango.
static void print_alert(uint8_t h, uint8_t m, uint8_t s, const char *tipo,
		        const char *periodo, uint8_t valor, const char *unidad) {
	uart_send_string("[ALERTA] [");
	uart_send_num2(h);
	uart_send_string(":");
	uart_send_num2(m);
	uart_send_string(":");
	uart_send_num2(s);
	uart_send_string("] ");
	uart_send_string(tipo);
	uart_send_string(" fuera de rango ");
	uart_send_string(periodo);
	uart_send_string("! Valor: ");
	uart_send_num2(valor);
	uart_send_string(unidad);
	uart_send_string("\r\n");
}

// Reinicia el estado funcional del modulo de aplicacion.
void app_init(void) {
	intervalo_muestreo = INTERVALO_MUESTREO_DEFAULT;
	contador_alerta_temp = 0;
	contador_alerta_hum = 0;
}

// Ejecuta la tarea periodica de muestreo y publicacion de telemetria.
void app_telemetry_task(void) {
	if (timer_tick()) {
		if (timer_get_elapsed_seconds() >= intervalo_muestreo) {
			timer_reset_elapsed();
			uint8_t rx_count = uart_get_rx_index();

			uint8_t h, m, s;
			rtc_read_time(&h, &m, &s);

			uint8_t temp, hum;
			if (dht11_read(&temp, &hum)) {
				// Determinar si es día o noche
				uint8_t es_dia = (h >= HORA_DIA_INICIO && h <= HORA_DIA_FIN);
				uint8_t temp_ok = 0, hum_ok = 0;
				const char *periodo_str = "";

				if (es_dia) {
					periodo_str = "diurno";
					temp_ok = (temp >= TEMP_DIA_MIN &&
						   temp <= TEMP_DIA_MAX);
					hum_ok = (hum >= HUM_DIA_MIN &&
						  hum <= HUM_DIA_MAX);
				} else {
					periodo_str = "nocturno";
					temp_ok = (temp >= TEMP_NOCHE_MIN &&
						   temp <= TEMP_NOCHE_MAX);
					hum_ok = (hum >= HUM_NOCHE_MIN &&
						  hum <= HUM_NOCHE_MAX);
				}

				// Salto de línea si el usuario estaba escribiendo
				if (rx_count > 0) {
					uart_send_string("\r\n");
				}

				// Imprimir telemetría
				print_telemetry(h, m, s, temp, hum,
					       temp_ok && hum_ok);

				// Imprimir alertas independientes
				if (!temp_ok) {
					contador_alerta_temp++;
					if (contador_alerta_temp % 2 == 0) {
						print_alert(h, m, s,
							    "Temperatura",
							    periodo_str, temp,
							    "C");
					}
				} else {
					contador_alerta_temp = 0;
				}

				if (!hum_ok) {
					contador_alerta_hum++;
					if (contador_alerta_hum % 2 == 0) {
						print_alert(h, m, s,
							    "Humedad",
							    periodo_str, hum,
							    "%");
					}
				} else {
					contador_alerta_hum = 0;
				}

				// Devolver prompt si estaba escribiendo
				if (rx_count > 0) {
					uart_send_string("> ");
					volatile char *cmd = uart_get_command();
					for (uint8_t i = 0; i < rx_count;
					     i++) {
						uart_send_char(cmd[i]);
					}
				}

			} else {
				// Error en sensor
				if (rx_count > 0) {
					uart_send_string("\r\n");
				}
				uart_send_string(
					"Error: No se detecta el sensor DHT11.\r\n");
				if (rx_count > 0) {
					uart_send_string("> ");
					volatile char *cmd = uart_get_command();
					for (uint8_t i = 0; i < rx_count;
					     i++) {
						uart_send_char(cmd[i]);
					}
				}
			}
		}
	}
}

// Interpreta comandos ASCII recibidos por UART.
void app_command_task(void) {
	if (uart_command_ready()) {
		volatile char *cmd = uart_get_command();
		uint8_t len = str_length(cmd);

		if (str_match_prefix(cmd, "SET TM=")) {
			if (len == 8 || len == 9) {
				uint8_t nuevo_t = 0;
				if (len == 8) {
					nuevo_t = cmd[7] - '0';
				} else {
					nuevo_t = (cmd[7] - '0') * 10 +
						  (cmd[8] - '0');
				}

				if (nuevo_t >= 2 && nuevo_t <= 60) {
					intervalo_muestreo = nuevo_t;
					uart_send_string(
						"Intervalo actualizado correctamente.\r\n");
				} else {
					uart_send_string(
						"Error: El intervalo debe ser entre 2 y 60 seg.\r\n");
				}
			} else {
				uart_send_string(
					"Error: Formato invalido para SET TM.\r\n");
			}
		} else if (str_match_prefix(cmd, "SET TIME=")) {
			if (len == 17 && cmd[11] == ':' && cmd[14] == ':') {
				uint8_t h = (cmd[9] - '0') * 10 +
					     (cmd[10] - '0');
				uint8_t m = (cmd[12] - '0') * 10 +
					     (cmd[13] - '0');
				uint8_t s = (cmd[15] - '0') * 10 +
					     (cmd[16] - '0');

				if (h <= 23 && m <= 59 && s <= 59) {
					rtc_write_time(h, m, s);
					uart_send_string(
						"Hora del RTC actualizada.\r\n");
				} else {
					uart_send_string(
						"Error: Hora invalida. Maximo 23:59:59.\r\n");
				}
			} else {
				uart_send_string(
					"Error: Use formato exacto SET TIME=HH:MM:SS\r\n");
			}
		} else if (len > 0) {
			uart_send_string("Comando desconocido.\r\n");
		}

		uart_clear_command();
	}
}
