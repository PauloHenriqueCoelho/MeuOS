#ifndef API_H
#define API_H

#include <stdint.h>

// --- CONSOLE ---
void os_print(const char* message);
void os_print_color(const char* message, uint8_t fg, uint8_t bg);
void os_clear_screen();
void os_set_cursor(int x, int y);

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

// Verifica se o clique ocorreu dentro da área do botão de fechar [X]
// Retorna 1 se clicou, 0 se não.
int os_window_close_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y);
int os_window_title_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y);

#endif