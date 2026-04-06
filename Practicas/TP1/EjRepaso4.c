#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // Biblioteca GNU para la funcion sleep()

void imprimir_bits_lsb(uint8_t n) {
    // Un byte tiene 8 bits, asi que iteramos 8 veces
    for (uint8_t i = 0; i < 8; i++) {
        // Desplazamos 'i' posiciones a la derecha y enmascaramos con 1
        uint8_t bit = (n >> i) & 1;
        
        printf("%u ", bit);
        
        // ¡TRUCO VITAL! 
        // En C, la consola a veces guarda el texto en un buffer y no lo muestra 
        // hasta que hay un salto de linea (\n). Con esto forzamos a que lo imprima YA.
        fflush(stdout); 
        
        // Pausamos la ejecucion por 1 segundo 
        sleep(1); 
    }
    printf("\n"); // Al final damos el salto de linea
}

int main() {
    // Numero de prueba: 13. 
    // En binario es 00001101. 
    // Empezando por el LSB (derecha a izquierda) deberia imprimir: 1 0 1 1 0 0 0 0
    uint8_t numero = 13; 
    
    printf("Imprimiendo los bits del numero %u (empezando por el LSB):\n", numero);
    imprimir_bits_lsb(numero);
    
    return 0;
}