/*
 * main.c - Controlador de Horno a Microondas
 * Materia: Circuitos Digitales y Microcontroladores (E305) - UNLP
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include <util/delay.h>

// El techo matemático máximo: 99 minutos y 59 segundos
#define MAX_SEGUNDOS 5999 

// --- 1. Definición de los Estados de la MEF ---
typedef enum {
    ESTADO_STANDBY,
    ESTADO_CONFIGURANDO,
    ESTADO_COCINANDO,
    ESTADO_PAUSADO,
    ESTADO_FINALIZADO
} EstadoMicroondas;

// Declaraciones de funciones
void KEYPAD_Init();
uint8_t KeypadUpdate(void);
uint8_t KEYPAD_Scan(uint8_t *pkey);
void TIMER1_Init();
void Mostrar_Tiempo_LCD();
void Actualizar_Digitos_Desde_Segundos();

// Matriz de teclado en formato telefónico
const uint8_t teclado[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

// --- Variables Globales ---
uint8_t digitos[4] = {0, 0, 0, 0};       
uint16_t total_segundos = 0;             
volatile uint8_t flag_un_segundo = 0;    
uint8_t contador_finalizado = 0;         
uint8_t visible_finalizado = 1;          
uint8_t cantidad_digitos = 0;            

// Imprime el arreglo de dígitos en formato MM:SS
void Mostrar_Tiempo_LCD() {
    LCDGotoXY(0, 0);
    LCDsendChar(digitos[0] + '0'); 
    LCDsendChar(digitos[1] + '0'); 
    LCDsendChar(':'); 
    LCDsendChar(digitos[2] + '0'); 
    LCDsendChar(digitos[3] + '0'); 
}

// Descompone los segundos totales asegurando que nunca rompan el formato MM:SS
void Actualizar_Digitos_Desde_Segundos() {
    uint8_t min_act = total_segundos / 60;
    uint8_t seg_act = total_segundos % 60;
    digitos[0] = min_act / 10;
    digitos[1] = min_act % 10;
    digitos[2] = seg_act / 10;
    digitos[3] = seg_act % 10;
}

int main(void)
{   	
    /* Inicialización de Periféricos */
    KEYPAD_Init();
    LCDinit();
    TIMER1_Init();
    
    // Actuadores
    DDRB |= (1<<PB5);            
    DDRC |= (1<<PC4) | (1<<PC5); 
    PORTB &= ~(1<<PB5);
    PORTC &= ~((1<<PC4) | (1<<PC5));

    EstadoMicroondas estado_actual = ESTADO_STANDBY;

    LCDclr();
    Mostrar_Tiempo_LCD(); 

    /* Loop Principal */
    while (1)
    {
	    uint8_t tecla;
	    uint8_t presiono = KEYPAD_Scan(&tecla);
	    
        switch (estado_actual) {
            
            case ESTADO_STANDBY:
                PORTB &= ~(1<<PB5);
                PORTC &= ~((1<<PC4) | (1<<PC5));
                
                if (presiono) {
                    if (tecla >= '0' && tecla <= '9') {
                        estado_actual = ESTADO_CONFIGURANDO;
                        digitos[0] = 0; digitos[1] = 0; digitos[2] = 0;
                        digitos[3] = tecla - '0';
                        cantidad_digitos = 1; 
                        Mostrar_Tiempo_LCD();
                    }
                    else if (tecla == 'C') {
                        estado_actual = ESTADO_COCINANDO;
                        total_segundos = 30; // Directo, no llega al techo
                        Actualizar_Digitos_Desde_Segundos();
                        Mostrar_Tiempo_LCD();
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Cocinando...    ", 16);
                        PORTB |= (1<<PB5); 
                        PORTC |= (1<<PC4); 
                    }
                }
                break;

            case ESTADO_CONFIGURANDO:
                if (presiono) {
                    // Tope de entrada de 4 teclas
                    if (tecla >= '0' && tecla <= '9') {
                        if (cantidad_digitos < 4) {
                            digitos[0] = digitos[1];
                            digitos[1] = digitos[2];
                            digitos[2] = digitos[3];
                            digitos[3] = tecla - '0';
                            cantidad_digitos++;
                            Mostrar_Tiempo_LCD();
                        }
                    }
                    else if (tecla == 'B') {
                        estado_actual = ESTADO_STANDBY;
                        digitos[0] = digitos[1] = digitos[2] = digitos[3] = 0;
                        cantidad_digitos = 0;
                        Mostrar_Tiempo_LCD();
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"                ", 16);
                    }
                    else if (tecla == 'A') {
                        uint8_t min = (digitos[0] * 10) + digitos[1];
                        uint8_t seg = (digitos[2] * 10) + digitos[3];
                        total_segundos = (min * 60) + seg;
                        
                        // APLICAMOS EL TECHO MATEMÁTICO AL INICIAR
                        if (total_segundos > MAX_SEGUNDOS) {
                            total_segundos = MAX_SEGUNDOS; 
                        }
                        
                        if (total_segundos > 0) {
                            estado_actual = ESTADO_COCINANDO;
                            cantidad_digitos = 0; 
                            // Actualizamos el display por si el techo corrigió un "99:99" a "99:59"
                            Actualizar_Digitos_Desde_Segundos(); 
                            Mostrar_Tiempo_LCD();
                            
                            LCDGotoXY(0, 1);
                            LCDstring((uint8_t*)"Cocinando...    ", 16);
                            PORTB |= (1<<PB5); 
                            PORTC |= (1<<PC4); 
                        }
                    }
                    else if (tecla == 'C') {
                        uint8_t min = (digitos[0] * 10) + digitos[1];
                        uint8_t seg = (digitos[2] * 10) + digitos[3];
                        total_segundos = (min * 60) + seg + 30;
                        
                        // APLICAMOS EL TECHO MATEMÁTICO EN EL +30
                        if (total_segundos > MAX_SEGUNDOS) {
                            total_segundos = MAX_SEGUNDOS;
                        }
                        
                        Actualizar_Digitos_Desde_Segundos();
                        Mostrar_Tiempo_LCD();
                        // Importante: no reseteamos 'cantidad_digitos' porque seguimos configurando
                    }
                }
                break;

            case ESTADO_COCINANDO:
                if (flag_un_segundo) {
                    flag_un_segundo = 0;
                    if (total_segundos > 0) {
                        total_segundos--;
                        Actualizar_Digitos_Desde_Segundos();
                        Mostrar_Tiempo_LCD();
                    }
                    
                    if (total_segundos == 0) {
                        estado_actual = ESTADO_FINALIZADO;
                        PORTB &= ~(1<<PB5); 
                        PORTC &= ~(1<<PC4); 
                        PORTC |= (1<<PC5);  
                        contador_finalizado = 0;
                        visible_finalizado = 1;
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Fin Coccion!    ", 16);
                    }
                }
                
                if (presiono) {
                    if (tecla == 'B') {
                        estado_actual = ESTADO_PAUSADO;
                        PORTB &= ~(1<<PB5); 
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Pausado...      ", 16);
                    }
                    else if (tecla == 'C') {
                        total_segundos += 30;
                        
                        // APLICAMOS EL TECHO MATEMÁTICO AL SUMAR TIEMPO EN CALIENTE
                        if (total_segundos > MAX_SEGUNDOS) {
                            total_segundos = MAX_SEGUNDOS;
                        }
                        
                        Actualizar_Digitos_Desde_Segundos();
                        Mostrar_Tiempo_LCD();
                    }
                    else if (tecla == 'D') {
                        estado_actual = ESTADO_PAUSADO;
                        PORTB &= ~(1<<PB5); 
                        PORTC &= ~(1<<PC4); 
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Puerta Abierta! ", 16);
                    }
                }
                break;

            case ESTADO_PAUSADO:
                if (presiono) {
                    if (tecla == 'A') {
                        estado_actual = ESTADO_COCINANDO;
                        PORTB |= (1<<PB5); 
                        PORTC |= (1<<PC4); 
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Cocinando...    ", 16);
                    } 
                    else if (tecla == 'B') {
                        estado_actual = ESTADO_STANDBY;
                        digitos[0] = digitos[1] = digitos[2] = digitos[3] = 0;
                        Mostrar_Tiempo_LCD();
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"                ", 16);
                    }
                }
                break;

            case ESTADO_FINALIZADO:
                if (flag_un_segundo) {
                    flag_un_segundo = 0;
                    contador_finalizado++;
                    visible_finalizado = !visible_finalizado; 
                    
                    if (visible_finalizado) {
                        Mostrar_Tiempo_LCD();
                        LCDGotoXY(0, 1);
                        LCDstring((uint8_t*)"Fin Coccion!    ", 16);
                        PORTC |= (1<<PC5);  
                    } else {
                        LCDclr();           
                        PORTC &= ~(1<<PC5); 
                    }
                    
                    if (contador_finalizado >= 5) {
                        estado_actual = ESTADO_STANDBY;
                        digitos[0] = digitos[1] = digitos[2] = digitos[3] = 0;
                        LCDclr();
                        Mostrar_Tiempo_LCD();
                    }
                }

                if (presiono) {
                    estado_actual = ESTADO_STANDBY;
                    PORTC &= ~(1<<PC5);
                    digitos[0] = digitos[1] = digitos[2] = digitos[3] = 0;
                    LCDclr();
                    Mostrar_Tiempo_LCD();
                }
                break;
        }
    }
}

// ==============================================================
// CONFIGURACIÓN Y RUTINAS DEL TIMER 1
// ==============================================================

void TIMER1_Init() {
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    OCR1A = 15624; 
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

ISR(TIMER1_COMPA_vect) {
    flag_un_segundo = 1; 
}

// ==============================================================
// FUNCIONES BAJO NIVEL DEL TECLADO MATRICIAL 4x4
// ==============================================================

void KEYPAD_Init(){
	DDRB |= (1<<PB4) | (1<<PB3) | (1<<PB0);
	DDRD |= (1<<PD7);
	DDRD &= ~((1<<PD3)|(1<<PD5)|(1<<PD4)|(1<<PD2));
	PORTD |= (1<<PD3)|(1<<PD5)|(1<<PD4)|(1<<PD2);
}

uint8_t KeypadUpdate(void)
{
	for(uint8_t f = 0; f < 4; f++) {
		PORTB |= (1<<PB4) | (1<<PB3) | (1<<PB0);
		PORTD |= (1<<PD7);

		switch(f)
		{
			case 0: PORTB &= ~(1<<PB4); break; 
			case 1: PORTB &= ~(1<<PB3); break; 
			case 2: PORTB &= ~(1<<PB0); break; 
			case 3: PORTD &= ~(1<<PD7); break; 
		}
		
		_delay_us(10);
		uint8_t col = 0xFF;

		if(!(PIND & (1<<PD3))) col = 0;
		else if(!(PIND & (1<<PD5))) col = 1;
		else if(!(PIND & (1<<PD4))) col = 2;
		else if(!(PIND & (1<<PD2))) col = 3;

		if(col != 0xFF) return teclado[f][col];
	}
	return 0xFF;
}

uint8_t KEYPAD_Scan(uint8_t *pkey)
{
	static uint8_t Old_key = 0xFF;
	static uint8_t Last_valid_key = 0xFF;

	uint8_t Key = KeypadUpdate();

	if(Key == 0xFF){
		Old_key = 0xFF;
		Last_valid_key = 0xFF;
		return 0;
	}

	if(Key == Old_key){
		if(Key != Last_valid_key){
			*pkey = Key;
			Last_valid_key = Key;
			return 1;
		}
	}

	Old_key = Key;
	return 0;
}