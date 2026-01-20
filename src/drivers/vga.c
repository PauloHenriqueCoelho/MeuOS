#include "../include/io.h"
#include <stdint.h>

// Constantes do VGA
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Portas do Cursor (Hardware)
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_OFFSET_LOW 0x0F
#define VGA_OFFSET_HIGH 0x0E

// Variáveis internas do driver
volatile uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
int terminal_col = 0;
int terminal_row = 0;
uint8_t terminal_color = 0x0F; // Branco no preto

// Função interna para mover o cursor físico (aquela barrinha piscando)
void vga_update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;

    outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}

// Limpa a tela
void vga_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
    }
    terminal_col = 0;
    terminal_row = 0;
    vga_update_cursor(0, 0);
}

// Imprime um caractere e trata nova linha
void vga_putchar(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else {
        int index = terminal_row * VGA_WIDTH + terminal_col;
        vga_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
        terminal_col++;
    }

    // Se chegar no fim da linha, pula
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }

    // (Opcional) Implementar scroll aqui depois se terminal_row >= VGA_HEIGHT

    vga_update_cursor(terminal_col, terminal_row);
}

// Imprime uma string
void vga_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

// ... (código anterior do vga.c) ...

// Remove o último caractere
void vga_backspace() {
    if (terminal_col > 0) {
        terminal_col--;
        int index = terminal_row * VGA_WIDTH + terminal_col;
        vga_buffer[index] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        vga_update_cursor(terminal_col, terminal_row);
    }
}  