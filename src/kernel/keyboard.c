#include "../include/io.h"
#include "../include/utils.h"
#include "../include/vga.h"

// Buffer Circular
#define KB_BUFFER_SIZE 256
char kb_buffer[KB_BUFFER_SIZE];
int write_ptr = 0;
int read_ptr = 0;

// Mapa de Teclado (US)
char kbd_US [128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// --- Chamado pelo ASSEMBLY (Automaticamente) ---
void keyboard_handler_isr() {
    uint8_t scancode = inb(0x60);

    // Se for tecla pressionada (bit 7 desligado)
    if (!(scancode & 0x80)) {
        char c = kbd_US[scancode];
        if (c != 0) {
            // Guarda no buffer
            kb_buffer[write_ptr] = c;
            write_ptr = (write_ptr + 1) % KB_BUFFER_SIZE;
        }
    }
}

// --- Chamado pelo SHELL (Quando quiser ler) ---
char keyboard_get_key() {
    if (read_ptr != write_ptr) {
        char c = kb_buffer[read_ptr];
        read_ptr = (read_ptr + 1) % KB_BUFFER_SIZE;
        return c;
    }
    return 0; // Buffer vazio
}

// Inicializa hardware
void keyboard_init() {
    write_ptr = 0;
    read_ptr = 0;
    
    // Ativa teclado
    outb(0x64, 0xAE);
    io_wait();
    outb(0x60, 0xF4);
    
    // Limpa lixo inicial
    for(int i=0; i<1000; i++) {
        if(inb(0x64) & 1) inb(0x60);
    }
}