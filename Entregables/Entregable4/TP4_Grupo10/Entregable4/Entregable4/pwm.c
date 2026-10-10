#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWM_PERIOD 256

static volatile uint8_t color_r = 0;
static volatile uint8_t color_g = 255;
static volatile uint8_t color_b = 255;
static volatile uint8_t intensidad_actual = 0;
static volatile uint8_t rojo_soft_pwm = 0;
static volatile uint16_t contador_pwm_soft = 0;

// Interrupcion del Timer 0: genera el PWM por software comparando con rojo_soft_pwm
ISR(TIMER0_OVF_vect) {
    if (++contador_pwm_soft >= PWM_PERIOD) {
        contador_pwm_soft = 0;
    }
    if (contador_pwm_soft < rojo_soft_pwm) {
        PORTB &= ~(1 << PB5); // Prende (0V)
    } else {
        PORTB |= (1 << PB5);  // Apaga (5V)
    }
}

// Configura ambos Timers (0 y 1) para funcionar a ~244Hz y asi evitar desfasajes visuales
void PWM_Init(void) {
    DDRB |= (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB5);
    PORTB |= (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB5); // Iniciar apagado (5V)

    TCCR1A |= (1 << WGM10); // Fast PWM, 8-bit
    TCCR1B |= (1 << WGM12);
    TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0);
    TCCR1B |= (1 << CS12);

    TCCR0B |= (1 << CS00); // Timer 0: Prescaler 1 (F_INT = 16M/256 = 62.5kHz)
    TIMSK0 |= (1 << TOIE0);
	
}

void PWM_SetColor(uint8_t r, uint8_t g, uint8_t b) {
    color_r = r;
    color_g = g;
    color_b = b;
}

void PWM_SetIntensity(uint8_t intensity) {
    intensidad_actual = intensity;
}

// Ajusta la intensidad de cada componente de color segun la intensidad global actual
void PWM_Update(void) {
    uint8_t azul_escalado = ((uint16_t)color_b * intensidad_actual) / 255;
    uint8_t verde_escalado = ((uint16_t)color_g * intensidad_actual) / 255;
    uint8_t rojo_escalado = ((uint16_t)color_r * intensidad_actual) / 255;

    OCR1A = azul_escalado;
    OCR1B = verde_escalado;
    rojo_soft_pwm = rojo_escalado;
}
