#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// Cores Padrão (Formato 0xAARRGGBB - Alpha, Red, Green, Blue)
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED   0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFF0000FF
#define COLOR_GRAY  0xFFC0C0C0
#define COLOR_DARK_BLUE 0xFF000080

// Inicialização
void vga_init_from_multiboot(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp);
void gfx_clear_screen(uint32_t color); // <--- FALTAVA ISSO AQUI!

// Primitivas Gráficas (32-bit)
void gfx_put_pixel(int x, int y, uint32_t color);
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_draw_char(int x, int y, char c, uint32_t color);
void gfx_draw_window(char* title, int x, int y, int w, int h, uint32_t body_color);
void gfx_draw_button(char* label, int x, int y, int w, int h, uint32_t color);

// Console Gráfico (Substituto do vga_print antigo)
void vga_set_cursor(int x, int y);
void vga_print(const char* str);
void vga_print_color(const char* str, uint32_t color);
void vga_clear();
void gfx_draw_cursor(int x, int y); // Mouse

// Getters
int get_screen_width();
int get_screen_height();

#endif