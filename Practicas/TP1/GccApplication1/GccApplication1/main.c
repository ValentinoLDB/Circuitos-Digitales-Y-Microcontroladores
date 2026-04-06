/*
 * GccApplication1.c
 *
 * Created: 9/3/2026 14:57:52
 * Author : Barcala
 */ 

#include <avr/io.h>


int main(void)
{
    DDRD=0b11111111;
    while (1) 
    {
		PORTD=0x55;
		PORTD=0xAA;
    }
}

