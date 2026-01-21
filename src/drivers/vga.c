#include "../include/vga.h"
#include "../include/font.h" 

static uint32_t* vga_buffer;
static int vga_width;
static int vga_height;
static int vga_pitch;
static int vga_bpp;

// Variáveis para o Cursor de Texto (Console)
static int cursor_x = 0;
static int cursor_y = 0;

void vga_init_from_multiboot(uint32_t addr, uint32_t w, uint32_t h, uint32_t p, uint32_t bpp) {
    vga_buffer = (uint32_t*)addr;
    if (w == 0) {
        vga_width = 1024;
        vga_height = 768;
        vga_pitch = 1024 * 4;
    } else {
        vga_width = (int)w;
        vga_height = (int)h;
        vga_pitch = (int)p;
    }
    vga_bpp = (int)bpp;
}

void gfx_put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= vga_width || y < 0 || y >= vga_height) return;
    vga_buffer[y * (vga_pitch / 4) + x] = color;
}

uint32_t gfx_get_pixel(int x, int y) {
    if (x < 0 || x >= vga_width || y < 0 || y >= vga_height) return 0;
    return vga_buffer[y * (vga_pitch / 4) + x];
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for(int i=0; i<w; i++) {
        for(int j=0; j<h; j++) {
            gfx_put_pixel(x+i, y+j, color);
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint32_t color) {
    if (x < 0 || y < 0) return;
    uint8_t* glyph = (uint8_t*)font8x8_basic[(int)((unsigned char)c)];
    for (int row = 0; row < 8; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                gfx_put_pixel(x + col, y + row, color);
            }
        }
    }
}

// --- FUNÇÕES DE CONSOLE (EMULAÇÃO DE TEXTO) ---
// Adicionadas para corrigir os erros do Shell, FS e RTC

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void vga_print_color(const char* str, uint32_t color) {
    while (*str) {
        if (*str == '\n') {
            cursor_x = 0;
            cursor_y += 10; // Pula linha (8px + 2px margem)
        } else {
            gfx_draw_char(cursor_x, cursor_y, *str, color);
            cursor_x += 8;
            
            // Quebra de linha automática se bater na borda
            if (cursor_x >= vga_width) {
                cursor_x = 0;
                cursor_y += 10;
            }
        }
        str++;
    }
}

void vga_print(const char* str) {
    vga_print_color(str, 0xFFFFFFFF); // Branco padrão
}

void vga_clear() {
    gfx_clear_screen(0xFF000000); // Limpa com preto
    cursor_x = 0;
    cursor_y = 0;
}
// ----------------------------------------------

void gfx_draw_window(char* title, int x, int y, int w, int h, uint32_t color) {
    gfx_fill_rect(x, y, w, h, color); 
    gfx_fill_rect(x, y, w, 20, 0xFF000080); 
    
    int title_x = x + 5;
    while (*title) {
        gfx_draw_char(title_x, y+4, *title, 0xFFFFFFFF);
        title_x += 8;
        title++;
    }
    
    gfx_fill_rect(x + w - 18, y + 4, 14, 12, 0xFFCC0000); 
    gfx_draw_char(x + w - 14, y + 6, 'X', 0xFFFFFFFF);
    
    gfx_fill_rect(x, y, w, 1, 0xFFFFFFFF);
    gfx_fill_rect(x, y, 1, h, 0xFFFFFFFF);
    gfx_fill_rect(x + w - 1, y, 1, h, 0xFF404040);
    gfx_fill_rect(x, y + h - 1, w, 1, 0xFF404040);
}

void gfx_draw_button(char* label, int x, int y, int w, int h, uint32_t color) {
    gfx_fill_rect(x, y, w, h, color);
    gfx_fill_rect(x, y, w, 1, 0xFFFFFFFF);
    gfx_fill_rect(x, y, 1, h, 0xFFFFFFFF);
    gfx_fill_rect(x+w-1, y, 1, h, 0xFF404040);
    gfx_fill_rect(x, y+h-1, w, 1, 0xFF404040);

    int len = 0; while(label[len]) len++;
    int text_x = x + (w - (len*8))/2;
    int text_y = y + (h - 8)/2;
    while(*label) {
        gfx_draw_char(text_x, text_y, *label, 0xFF000000);
        text_x += 8;
        label++;
    }
}

void gfx_clear_screen(uint32_t color) {
    int total = (vga_pitch / 4) * vga_height;
    for (int i=0; i<total; i++) vga_buffer[i] = color;
}

int get_screen_width() { return vga_width; }
int get_screen_height() { return vga_height; }