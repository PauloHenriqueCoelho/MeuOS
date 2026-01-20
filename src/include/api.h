#ifndef API_H
#define API_H

#include <stdint.h>

// --- CONSOLE ---
void os_print(const char* message);
void os_print_color(const char* message, uint8_t fg, uint8_t bg);
void os_clear_screen();
void os_set_cursor(int x, int y); // <--- Adicionado

// --- ENTRADA ---
char os_wait_for_key();

// --- ARQUIVOS ---
int os_file_exists(char* name);
int os_file_create(char* name, char* content);
int os_file_read(char* name, char* buffer);
int os_file_delete(char* name);
void os_file_list();

// --- SISTEMA / GUI ---
void os_reboot();
void os_create_window(char* title, int x, int y, int w, int h, uint8_t color);

#endif