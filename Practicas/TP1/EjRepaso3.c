#include <stdio.h>
#include <stdint.h>

// La función recibe el número y un puntero a un arreglo (buffer) donde guardaremos los caracteres
void bin_to_ascii(uint8_t num, char *str) {
    uint8_t indice = 0;
    
    // Evaluamos las centenas (si el número es mayor o igual a 100)
    if (num >= 100) {
        str[indice++] = (num / 100) + '0'; // Obtenemos la centena y la pasamos a ASCII
        num %= 100;                        // Nos quedamos con el resto (decenas y unidades)
        str[indice++] = (num / 10) + '0';  // Obtenemos la decena y la pasamos a ASCII
    } 
    // Evaluamos las decenas (si el número original era menor a 100 pero mayor a 9)
    else if (num >= 10) {
        str[indice++] = (num / 10) + '0';  // Obtenemos la decena y la pasamos a ASCII
    }
    
    // Las unidades se calculan siempre
    str[indice++] = (num % 10) + '0';      // Obtenemos la unidad y la pasamos a ASCII
    
    // ¡Fundamental! Cerramos la cadena de caracteres con el terminador nulo
    str[indice] = '\0';
}

int main() {
    unsigned int entrada; // Usamos int para el scanf, luego lo pasamos a 8 bits
    uint8_t numero_8bits;
    char cadena_resultado[4]; // Arreglo de 4 posiciones (hasta 3 dígitos + el '\0')

    // Usamos la interfaz de entrada estándar como pide el TP
    printf("Ingresa un numero entre 0 y 255: ");
    scanf("%u", &entrada);

    // Protegemos el programa por si el usuario pone un número más grande
    if (entrada > 255) {
        printf("Error: El numero debe ser de 8 bits (maximo 255).\n");
        return 1;
    }

    numero_8bits = (uint8_t)entrada;

    // Llamamos a la funcion
    bin_to_ascii(numero_8bits, cadena_resultado);

    // Imprimimos el resultado
    printf("El numero convertido a caracteres ASCII es: %s\n", cadena_resultado);

    return 0;
}