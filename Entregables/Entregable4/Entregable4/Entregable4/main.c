#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "adc.h"
#include "uart.h"
#include "temporizacion.h"

int main(void) {
    PWM_Init();
    ADC_Init();
    UART_Init();
    Temporizacion_Init();

    sei();

    UART_SendString("Control LED RGB iniciado.\r\n");
    UART_SendString("Formato de comando: SET_COLOR=X,X,X   (0 <= X <= 255)\r\n");

    uint32_t periodo_T = 6000; // Inicia lento (minima luz)
    uint32_t ultimo_lectura_adc = 0;

    while (1) {
        UART_ParseCommand();
        
        // Leemos el LDR cada 200ms para no bloquear la simulacion con lecturas continuas del ADC
        uint32_t ms_actual = Temporizacion_GetMillis();
        if ((ms_actual - ultimo_lectura_adc) > 200) {
            periodo_T = ADC_GetPeriodoT();
            ultimo_lectura_adc = ms_actual;
        }
        
        Temporizacion_AnimacionUpdate(periodo_T);
    }

    return 0;
}
