#include "adc.h"
#include <avr/io.h>

static uint16_t leer_ADC(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void ADC_Init(void) {
    ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint32_t ADC_GetPeriodoT(void) {
    uint16_t valor_adc = leer_ADC();
    uint32_t periodo = 6000 - ((uint32_t)valor_adc * 3000 / 1023);

    if (periodo < 3000) periodo = 3000;
    if (periodo > 6000) periodo = 6000;

    return periodo;
}
