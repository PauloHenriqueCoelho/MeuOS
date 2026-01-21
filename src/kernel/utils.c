#include "../include/utils.h"

void memory_copy(char* source, char* dest, int n) {
    for (int i = 0; i < n; i++) {
        *(dest + i) = *(source + i);
    }
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    // Inverte
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// --- NOVAS IMPLEMENTAÇÕES ---

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

void strcat(char* dest, const char* src) {
    // 1. Vai até o final da string de destino (onde está o '\0')
    char* d = dest;
    while (*d != '\0') {
        d++;
    }

    // 2. Começa a copiar a string de origem para o final da de destino
    while (*src != '\0') {
        *d = *src;
        d++;
        src++;
    }

    // 3. Adiciona o novo terminador nulo no final de tudo
    *d = '\0';
}