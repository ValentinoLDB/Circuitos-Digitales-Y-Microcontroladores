#include <stdio.h>
#include <stdint.h> // Para usar los tipos exactos como uint8_t y uint16_t

// Función pedida
uint16_t suma_consecutivos(uint8_t n) {
    uint16_t suma = 0;
    
    // Bucle para sumar desde 1 hasta N
    for (uint16_t i = 1; i <= n; i++) {
        suma += i;
    }
    
    return suma;
}

// Programa principal para ensayar la función
int main() {
    uint8_t N = 255; // Probamos con el límite máximo
    
    uint16_t resultado = suma_consecutivos(N);
    
    // Imprimimos el resultado en la salida estándar
    printf("La suma de los numeros de 1 hasta %u es: %u\n", N, resultado);
    
    return 0;
}