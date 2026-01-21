#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void memory_copy(char* source, char* dest, int n);
void int_to_ascii(int n, char str[]);
int strlen(const char* str);

// --- ADICIONE ESTAS ---
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);
void strcpy(char* dest, const char* src);
void strcat(char* dest, const char* src);
void* memset(void* ptr, int value, int num);

#endif