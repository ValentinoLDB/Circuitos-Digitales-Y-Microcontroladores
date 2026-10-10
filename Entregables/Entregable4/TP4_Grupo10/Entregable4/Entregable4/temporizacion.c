#include "temporizacion.h"
#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t ms_totales = 0;

// Timer 2 configurado a 1ms
ISR(TIMER2_COMPA_vect) {
    ms_totales++;
}

void Temporizacion_Init(void) {
    TCCR2A |= (1 << WGM21);
    TCCR2B |= (1 << CS22);
    OCR2A = 249; // Con prescaler 64, 16MHz/64 = 250kHz (4us/tick). 250 ticks * 4us = 1ms. Top = 250-1 = 249.
    TIMSK2 |= (1 << OCIE2A);
}

uint32_t Temporizacion_GetMillis(void) {
    uint32_t ms;
    cli(); // Deshabilita interrupciones para una lectura atomica de los 32 bits (4 bytes) y evitar corrupcion
    ms = ms_totales;
    sei();
    return ms;
}

void Temporizacion_AnimacionUpdate(uint32_t periodo_T) {
    static uint32_t ms_inicio_ciclo = 0;
    
    uint32_t ms_actual = Temporizacion_GetMillis();
    uint32_t ms_transcurridos = ms_actual - ms_inicio_ciclo;

    if (ms_transcurridos >= periodo_T) {
		TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0); // Reconecta PB1/PB2 al hardware PWM para iniciar nuevo ciclo de luz
        ms_inicio_ciclo = ms_actual;
        ms_transcurridos = 0;
    }

    uint8_t intensidad_actual;
    if (ms_transcurridos <= 1000) {
        intensidad_actual = ((uint32_t)ms_transcurridos * 255) / 1000; // Subida lineal
    } else if (ms_transcurridos <= 2000) {
        intensidad_actual = 255; // Pico maximo
    } else if (ms_transcurridos <= 3000) {
        uint32_t ms_en_bajada = ms_transcurridos - 2000;
        intensidad_actual = 255 - ((ms_en_bajada * 255) / 1000); // Bajada lineal
    } else if(ms_transcurridos<periodo_T) {
        intensidad_actual = 0; // Tiempo muerto
		TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0)); // Desconecta el hardware PWM de PB1/PB2 para apagar todo durante el tiempo muerto
		PORTB |= ((1<<PORTB1)|(1<<PORTB2));
    }

    PWM_SetIntensity(intensidad_actual);
    PWM_Update();
}
