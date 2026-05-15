/*
 * Display.c
 */ 

#include <avr/io.h>
#include "lcd.h"
#include <util/delay.h>

void KEYPAD_Init();
uint8_t KeypadUpdate(void);
uint8_t KEYPAD_Scan(uint8_t *pkey);

// Matriz restaurada al formato del TP2 (Figura 1)
const uint8_t teclado[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

int main(void)
{   	
    /* Setup */
    KEYPAD_Init();
    LCDinit();
    LCDGotoXY(0,0);
	//LCDsendChar('a');

    /* Loop */
    while (1)
    {
	    uint8_t tecla;
	    uint8_t presiono = KEYPAD_Scan(&tecla);
	    
	    if (presiono)
	    {
		    LCDGotoXY(0,1);
		    LCDsendChar(tecla);	// escribo 1 caracter ASCII
	    }
    }
}

void KEYPAD_Init(){
	// Filas como salida
	DDRB |= (1<<PB4) | (1<<PB3) | (1<<PB0);
	DDRD |= (1<<PD7);

	// Columnas como entrada + pull-up
	DDRD &= ~((1<<PD3)|(1<<PD5)|(1<<PD4)|(1<<PD2));
	PORTD |= (1<<PD3)|(1<<PD5)|(1<<PD4)|(1<<PD2);
}

uint8_t KeypadUpdate(void)
{
	// Escaneamos TODAS las filas en una sola pasada
	for(uint8_t f = 0; f < 4; f++) {
		
		// Poner todas las filas en 1
		PORTB |= (1<<PB4) | (1<<PB3) | (1<<PB0);
		PORTD |= (1<<PD7);

		// Activar la fila actual poniendo un 0
		switch(f)
		{
			case 0: PORTB &= ~(1<<PB4); break; // A
			case 1: PORTB &= ~(1<<PB3); break; // B
			case 2: PORTB &= ~(1<<PB0); break; // C
			case 3: PORTD &= ~(1<<PD7); break; // D
		}
		
		// Micro-delay necesario tanto en físico como en simulación para estabilizar la lectura
		_delay_us(10);

		uint8_t col = 0xFF;

		if(!(PIND & (1<<PD3))) col = 0;
		else if(!(PIND & (1<<PD5))) col = 1;
		else if(!(PIND & (1<<PD4))) col = 2;
		else if(!(PIND & (1<<PD2))) col = 3;

		// Si detecta una columna en esta fila, devuelve la tecla inmediatamente
		if(col != 0xFF)
		{
			return teclado[f][col];
		}
	}

	// Si dio la vuelta por todas las filas y no encontró nada
	return 0xFF;
}

uint8_t KEYPAD_Scan(uint8_t *pkey)
{
	static uint8_t Old_key = 0xFF;
	static uint8_t Last_valid_key = 0xFF;

	uint8_t Key;

	Key = KeypadUpdate();

	// No hay tecla
	if(Key == 0xFF){
		Old_key = 0xFF;
		Last_valid_key = 0xFF;
		return 0;
	}

	// Segunda verificación (antirrebote)
	if(Key == Old_key){
		
		// Evita múltiples detecciones mientras mantengo presionado
		if(Key != Last_valid_key){
			*pkey = Key;
			Last_valid_key = Key;
			return 1;
		}
	}

	// Primera verificación
	Old_key = Key;

	return 0;
}