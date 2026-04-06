#include <stdio.h>
#include <stdint.h>

// La función recibe el arreglo por referencia (puntero *) y el tamaño por valor
void invertir_arreglo(uint8_t *arr, uint8_t size) {
    uint8_t temp;
    uint8_t inicio = 0;
    uint8_t fin = size - 1;

    // Si el arreglo está vacío, salimos para evitar errores
    if (size == 0) return; 

    // Vamos intercambiando los extremos hacia el centro
    while (inicio < fin) {
        // Guardamos el valor inicial temporalmente (clásico algoritmo de swap)
        temp = arr[inicio];
        
        // El del final pasa al inicio
        arr[inicio] = arr[fin];
        
        // El temporal (que era el inicio) pasa al final
        arr[fin] = temp;

        // Movemos los índices hacia el centro
        inicio++;
        fin--;
    }
}

// Función auxiliar para imprimir el arreglo y ver los resultados
void imprimir_arreglo(uint8_t *arr, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Definimos nuestro arreglo de prueba
    uint8_t mi_vector[] = {10, 20, 30, 40, 50, 60};
    
    // Calculamos el tamaño: tamaño total en bytes / tamaño de 1 elemento en bytes
    uint8_t tamano = sizeof(mi_vector) / sizeof(mi_vector[0]);

    printf("Arreglo original: ");
    imprimir_arreglo(mi_vector, tamano);

    // Llamamos a la función
    invertir_arreglo(mi_vector, tamano);

    printf("Arreglo invertido: ");
    imprimir_arreglo(mi_vector, tamano);

    return 0;
}