#include "../include/mouse.h"
#include "../include/io.h"
#include "../include/vga.h" 

// --- CONFIGURAÇÃO DO CURSOR ---
#define CURSOR_WIDTH  8
#define CURSOR_HEIGHT 12

// Mapa de bits do cursor (Triângulo Simples)
// 0 = Transparente
// 1 = Borda Preta
// 2 = Preenchimento Branco
uint8_t mouse_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {1,0,0,0,0,0,0,0}, // Ponta
    {1,1,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0},
    {1,2,2,1,0,0,0,0},
    {1,2,2,2,1,0,0,0},
    {1,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,0},
    {1,1,1,1,1,1,1,1}, // Base do triângulo
    {0,0,0,0,0,0,0,0}, // Sem "perninha" abaixo
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
};

// Variáveis de Estado
uint8_t mouse_cycle = 0;     
int8_t  mouse_byte[3];       
int     mouse_x = 160;       
int     mouse_y = 100;       
int     mouse_buttons = 0;   

// Buffer de restauração
#define BUFFER_W 10
#define BUFFER_H 14
uint8_t mouse_bg_buffer[BUFFER_W * BUFFER_H]; 
int     mouse_last_x = 160;
int     mouse_last_y = 100;
int     first_draw = 1;
int     background_invalid = 1;

// --- FUNÇÕES AUXILIARES ---
void mouse_wait(uint8_t type) {
    uint32_t time_out = 100000;
    if (type == 0) { 
        while (time_out--) { if ((inb(0x64) & 1) == 1) return; }
    } else { 
        while (time_out--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1); outb(0x64, 0xD4); mouse_wait(1); outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait(0); return inb(0x60);
}

// Reseta o background (Previne manchas na troca de tela)
void mouse_reset_background() {
    background_invalid = 1;
}

// --- DESENHO DO CURSOR ---
// No arquivo src/drivers/mouse.c

void draw_mouse_cursor() {
    uint8_t* vga = (uint8_t*)0xA0000;

    // 1. RESTAURA o fundo antigo (apaga o mouse da posição anterior)
    if (!first_draw && !background_invalid) {
        for(int y=0; y<BUFFER_H; y++) {
            for(int x=0; x<BUFFER_W; x++) {
                int old_screen_x = mouse_last_x + x;
                int old_screen_y = mouse_last_y + y;
                if (old_screen_x < 320 && old_screen_y < 200) {
                    vga[old_screen_y * 320 + old_screen_x] = mouse_bg_buffer[y*BUFFER_W + x];
                }
            }
        }
    }

    // 2. SALVA o novo fundo (onde o mouse está agora)
    for(int y=0; y<BUFFER_H; y++) {
        for(int x=0; x<BUFFER_W; x++) {
            int new_screen_x = mouse_x + x;
            int new_screen_y = mouse_y + y;
            if (new_screen_x < 320 && new_screen_y < 200) {
                mouse_bg_buffer[y*BUFFER_W + x] = vga[new_screen_y * 320 + new_screen_x];
            }
        }
    }

    // 3. DESENHA o cursor por cima
    for(int y=0; y<CURSOR_HEIGHT; y++) {
        for(int x=0; x<CURSOR_WIDTH; x++) {
            uint8_t pixel = mouse_bitmap[y][x];
            if (pixel == 1) vga_plot_pixel(mouse_x + x, mouse_y + y, 0);  // Borda
            else if (pixel == 2) vga_plot_pixel(mouse_x + x, mouse_y + y, 15); // Interior
        }
    }

    mouse_last_x = mouse_x;
    mouse_last_y = mouse_y;
    background_invalid = 0;
    first_draw = 0;
}
// --- HANDLER ---
void mouse_handler_isr() {
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) return; 
    uint8_t data = inb(0x60);

    switch(mouse_cycle) {
        case 0:
            if ((data & 0x08) == 0) return; 
            mouse_byte[0] = data; mouse_cycle++; break;
        case 1:
            mouse_byte[1] = data; mouse_cycle++; break;
        case 2:
            mouse_byte[2] = data;
            int8_t dx = mouse_byte[1];
            int8_t dy = -mouse_byte[2]; 
            mouse_x += dx;
            mouse_y += dy;
            
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x > 320 - CURSOR_WIDTH) mouse_x = 320 - CURSOR_WIDTH;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y > 200 - CURSOR_HEIGHT) mouse_y = 200 - CURSOR_HEIGHT;

            mouse_buttons = mouse_byte[0] & 0x07;
            mouse_cycle = 0; break;
    }
}

void mouse_init() {
    uint8_t status;
    mouse_wait(1); outb(0x64, 0xA8);
    mouse_wait(1); outb(0x64, 0x20);
    mouse_wait(0); status = (inb(0x60) | 2);
    mouse_wait(1); outb(0x64, 0x60);
    mouse_wait(1); outb(0x60, status);
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
    draw_mouse_cursor();
}

// Getters
int mouse_get_x() { return mouse_x; }
int mouse_get_y() { return mouse_y; }
int mouse_get_status() { return mouse_buttons; }