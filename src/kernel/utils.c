#include <stdint.h>

// Calcula o tamanho de uma string
int strlen(const char* str) {
    int len = 0;
    while (str[len])
        len++;
    return len;
}

// Compara duas strings
// Retorna 0 se forem iguais
// Retorna >0 ou <0 se forem diferentes
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Copia uma string (útil para mover dados)
void strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}