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

void gfx_fill_rect(int x, int y, int w, int h, uint8_t color);
void gfx_draw_char(int x, int y, char c, uint8_t color);

// API Gráfica
void gfx_draw_window(char* title, int x, int y, int w, int h, uint8_t body_color);
void gfx_put_pixel(int x, int y, uint8_t color);
void gfx_draw_button(char* label, int x, int y, int w, int h, uint8_t color);
void vga_print_at(int x, int y, char* str, uint8_t color);
void gfx_draw_cursor(int x, int y);
void vga_plot_pixel(int x, int y, uint8_t color);

#endif