#include "temporizacion.h"
#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t ms_totales = 0;

ISR(TIMER2_COMPA_vect) {
    ms_totales++;
}

void Temporizacion_Init(void) {
    TCCR2A |= (1 << WGM21);
    TCCR2B |= (1 << CS22);
    OCR2A = 249;
    TIMSK2 |= (1 << OCIE2A);
}

uint32_t Temporizacion_GetMillis(void) {
    uint32_t ms;
    cli();
    ms = ms_totales;
    sei();
    return ms;
}

void Temporizacion_AnimacionUpdate(uint32_t periodo_T) {
    static uint32_t ms_inicio_ciclo = 0;
    
    uint32_t ms_actual = Temporizacion_GetMillis();
    uint32_t ms_transcurridos = ms_actual - ms_inicio_ciclo;

    if (ms_transcurridos >= periodo_T) {
        ms_inicio_ciclo = ms_actual;
        ms_transcurridos = 0;
    }

    uint8_t intensidad_actual;
    if (ms_transcurridos <= 1000) {
        intensidad_actual = ((uint32_t)ms_transcurridos * 255) / 1000;
    } else if (ms_transcurridos <= 2000) {
        intensidad_actual = 255;
    } else if (ms_transcurridos <= 3000) {
        uint32_t ms_en_bajada = ms_transcurridos - 2000;
        intensidad_actual = 255 - ((ms_en_bajada * 255) / 1000);
    } else {
        intensidad_actual = 0;
    }

    PWM_SetIntensity(intensidad_actual);
    PWM_Update();
}
