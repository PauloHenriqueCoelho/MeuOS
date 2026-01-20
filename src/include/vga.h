#ifndef VGA_H
#define VGA_H
#include <stdint.h>

void video_init();
void vga_clear();
void vga_print(const char* str);
void vga_putchar(char c);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_backspace(); 

// Função que faltava declarar:
void vga_set_cursor(int x, int y);

// API Gráfica
void gfx_draw_window(char* title, int x, int y, int w, int h, uint8_t body_color);

#endif