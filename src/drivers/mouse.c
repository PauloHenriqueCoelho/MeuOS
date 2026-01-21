#include "../include/mouse.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/idt.h" // Para interrupções

// Constantes de Cor 32-bit
#define COLOR_MOUSE_BORDER 0xFF000000 // Preto
#define COLOR_MOUSE_FILL   0xFFFFFFFF // Branco

uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];
int mouse_x = 400; // Começa no meio (800/2)
int mouse_y = 300; // Começa no meio (600/2)
int mouse_status = 0;

// Cursor padrão (Bitmap simples 12x19)
// 1 = Borda (Preto), 2 = Preenchimento (Branco), 0 = Transparente
static int mouse_cursor[19][12] = {
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,1,1,1,0,0},
    {1,2,2,2,1,2,1,0,0,0,0,0},
    {1,2,1,1,0,1,2,1,0,0,0,0},
    {1,1,0,0,0,1,2,1,0,0,0,0},
    {0,0,0,0,0,0,1,2,1,0,0,0},
    {0,0,0,0,0,0,1,2,1,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
        return;
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
        return;
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_handler_isr() {
    uint8_t status = inb(0x64);
    if (!(status & 1)) return; // Buffer vazio

    uint8_t mouse_in = inb(0x60);
    
    // Sincronização básica
    if (mouse_cycle == 0 && !(mouse_in & 0x08)) return;

    mouse_byte[mouse_cycle++] = mouse_in;

    if (mouse_cycle >= 3) {
        mouse_cycle = 0;
        
        // Byte 0: Flags (Botões, overflow, sinais)
        // Byte 1: Delta X
        // Byte 2: Delta Y
        
        int dx = mouse_byte[1];
        int dy = mouse_byte[2];

        // Ajuste de sinal (9-bit)
        if (mouse_byte[0] & 0x10) dx |= 0xFFFFFF00; 
        if (mouse_byte[0] & 0x20) dy |= 0xFFFFFF00;

        mouse_x += dx;
        mouse_y -= dy; // Y é invertido no hardware PS/2

        // Proteção de bordas (800x600)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x >= get_screen_width() - 1) mouse_x = get_screen_width() - 1;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y >= get_screen_height() - 1) mouse_y = get_screen_height() - 1;

        mouse_status = mouse_byte[0];
    }
}

void mouse_init() {
    uint8_t status;

    // Habilita dispositivo auxiliar (mouse)
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Habilita interrupções
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // Usa configurações padrão
    mouse_write(0xF6);
    mouse_read();

    // Habilita envio de pacotes
    mouse_write(0xF4);
    mouse_read();
}

// --- DESENHO DO MOUSE (Corrigido para 32-bit) ---
void draw_mouse_cursor() {
    // Desenha o bitmap definido acima
    for(int y = 0; y < 19; y++) {
        for(int x = 0; x < 12; x++) {
            int pixel = mouse_cursor[y][x];
            
            if (pixel == 1) {
                // Borda Preta (0xFF000000)
                gfx_put_pixel(mouse_x + x, mouse_y + y, COLOR_MOUSE_BORDER);
            }
            else if (pixel == 2) {
                // Preenchimento Branco (0xFFFFFFFF)
                gfx_put_pixel(mouse_x + x, mouse_y + y, COLOR_MOUSE_FILL);
            }
            // Pixel 0 é transparente, não desenha nada
        }
    }
}

// --- API ---
int mouse_get_x() { return mouse_x; }
int mouse_get_y() { return mouse_y; }
int mouse_get_status() { return mouse_status; }

// Reseta o fundo (se necessário para lógica de double buffer futura)
void mouse_reset_background() {
    // Por enquanto não fazemos nada complexo aqui para manter simples
}