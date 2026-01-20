#ifndef API_H
#define API_H

#include <stdint.h>

// --- API DE CONSOLE (Tela) ---
void os_print(const char* message);
void os_print_color(const char* message, uint8_t fg, uint8_t bg);
void os_clear_screen();

// --- API DE ENTRADA (Teclado) ---
// Retorna a próxima tecla pressionada (bloqueante ou polling)
char os_wait_for_key();

// --- API DE ARQUIVOS (Disco) ---
// Retorna 1 se sucesso, 0 se falha
int os_file_exists(char* name);
int os_file_create(char* name, char* content);
int os_file_read(char* name, char* buffer); // buffer deve ter 512 bytes
int os_file_delete(char* name);
void os_file_list(); // Imprime a lista (simplificado)

// --- API DE SISTEMA ---
void os_reboot();
void os_create_window(char* title, int x, int y, int w, int h, uint8_t color);

#endif