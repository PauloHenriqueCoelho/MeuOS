#include "../include/io.h"
#include "../include/utils.h"
#include "../include/font.h" 

// Memória de vídeo no modo 13h (Linear)
uint8_t* VGA_MEMORY = (uint8_t*)0xA0000;

#define VGA_WIDTH 320
#define VGA_HEIGHT 200

// Cursor
int cursor_x = 0;
int cursor_y = 0;
uint8_t current_color = 15; // Branco
uint8_t bg_color = 3;       // Azul

// Escreve nos registros do VGA
void write_regs(uint8_t *regs) {
    uint8_t i;
    uint8_t temp; 

    // MISC
    outb(0x3C2, *regs); regs++;
    // SEQUENCER
    for(i = 0; i < 5; i++) {
        outb(0x3C4, i);
        outb(0x3C5, *regs); regs++;
    }
    
    // CRTC (Correção do inb/outb)
    outb(0x3D4, 0x03); 
    temp = inb(0x3D5);
    outb(0x3D5, temp | 0x80);

    outb(0x3D4, 0x11); 
    temp = inb(0x3D5);
    outb(0x3D5, temp & ~0x80);

    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;

    for(i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, *regs); regs++;
    }
    // GRAPHICS
    for(i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, *regs); regs++;
    }
    // ATTRIBUTE
    for(i = 0; i < 21; i++) {
        inb(0x3DA); 
        outb(0x3C0, i);
        outb(0x3C0, *regs); regs++;
    }
    inb(0x3DA);
    outb(0x3C0, 0x20);
}

// Configuração para 320x200x256
void vga_set_mode_13h() {
    uint8_t regs[] = {
        0x63, 
        0x03, 0x01, 0x0F, 0x00, 0x0E, 
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF, 
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 
        0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00 
    };
    write_regs(regs);
}

// --- PRIMITIVAS DE DESENHO ---

void gfx_put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        VGA_MEMORY[y * VGA_WIDTH + x] = color;
    }
}

void gfx_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, color);
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint8_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc > 127) return; 
    
    for (int i = 0; i < 8; i++) {
        uint8_t row = font8x8[uc][i];
        for (int j = 0; j < 8; j++) {
            if (row & (0x80 >> j)) {
                gfx_put_pixel(x + j, y + i, color);
            }
        }
    }
}

// --- INTEGRAÇÃO COM PRINTF ---

void vga_clear() {
    gfx_fill_rect(0, 0, VGA_WIDTH, VGA_HEIGHT, bg_color);
    cursor_x = 0;
    cursor_y = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 8;
    } else if (c == '\b') {
        // Lógica de Backspace Visual
        if (cursor_x >= 8) {
            cursor_x -= 8;
            gfx_fill_rect(cursor_x, cursor_y, 8, 8, bg_color);
        }
    } else {
        gfx_draw_char(cursor_x, cursor_y, c, current_color);
        cursor_x += 8;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y += 8;
    }
    if (cursor_y >= VGA_HEIGHT) {
        vga_clear();
    }
}

// --- CORREÇÃO: Implementação de Backspace para o Main ---
void vga_backspace() {
    vga_putchar('\b');
}

void vga_print(const char* str) {
    while(*str) {
        vga_putchar(*str++);
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = fg;
    (void)bg; 
}

// --- GUI ---
void gfx_draw_window(char* title, int x, int y, int w, int h, uint8_t body_color) {
    // 1. Corpo
    gfx_fill_rect(x, y, w, h, body_color);
    
    // 2. Barra
    gfx_fill_rect(x, y, w, 12, 8); // Cinza
    
    // 3. Título
    int title_len = strlen(title);
    int title_x = x + (w - (title_len * 8)) / 2;
    for(int i=0; i < title_len; i++) {
        // CORREÇÃO AQUI: Removemos o 5º argumento (cor de fundo), 
        // pois gfx_draw_char só aceita 4.
        gfx_draw_char(title_x + (i*8), y + 2, title[i], 15);
    }

    // 4. Botão X
    gfx_fill_rect(x + w - 10, y + 2, 8, 8, 4);
    gfx_draw_char(x + w - 10, y + 2, 'X', 15);

    // 5. Bordas
    for(int i=x; i<x+w; i++) { gfx_put_pixel(i, y, 15); gfx_put_pixel(i, y+h, 15); }
    for(int i=y; i<y+h; i++) { gfx_put_pixel(x, i, 15); gfx_put_pixel(x+w, i, 15); }
}

void video_init() {
    vga_set_mode_13h();
    vga_clear();
}