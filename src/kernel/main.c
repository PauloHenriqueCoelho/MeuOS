#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/shell.h"

// --- MAPA DE TECLADO (US) ---
char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// --- INICIALIZAÇÃO DE HARDWARE (A CORREÇÃO) ---
void keyboard_init() {
    // 1. Ativa a porta PS/2 na placa-mãe
    outb(0x64, 0xAE);
    
    // Pequeno delay
    for(volatile int i=0; i<1000; i++) {}

    // 2. Manda o comando "LIGAR ESCANEAMENTO" (0xF4) para o teclado
    // Sem isso, o teclado fica mudo (que era o seu problema no polling anterior)
    while((inb(0x64) & 0x2)) {} // Espera buffer limpar
    outb(0x60, 0xF4); 

    // 3. O teclado responde com ACK (0xFA). Vamos limpar isso.
    for(int i=0; i<10000; i++) {
        if(inb(0x64) & 0x1) {
            inb(0x60); // Joga fora o ACK
            break;
        }
    }
}

// Leitura manual (Não causa Triple Fault)
char keyboard_read_polling() {
    unsigned char status = inb(0x64);
    
    // Se o bit 0 for 1, tem dados!
    if (status & 0x01) {
        unsigned char scancode = inb(0x60);
        
        // Bit 7 desligado = Tecla Pressionada
        if (!(scancode & 0x80)) {
            if (scancode < 128) return kbd_US[scancode];
        }
    }
    return 0;
}

void kernel_main() {
    vga_clear();
    
    // --- REMOVEMOS init_gdt() e init_idt() DAQUI ---
    // Eles estavam causando o "CPU Reset" e tela preta.
    
    vga_print("=== MyOS Shell (Modo Polling + Hardware Fix) ===\n");
    vga_print("Inicializando teclado...\n");
    
    keyboard_init(); // Ativa o hardware corretamente
    
    vga_print("Teclado Ativo. Digite 'ajuda' e aperte ENTER:\n");
    vga_print("> ");

    char buffer[256];
    int index = 0;
    for(int i=0; i<256; i++) buffer[i] = 0;

    while(1) {
        char c = keyboard_read_polling();

        if (c != 0) {
            if (c == '\n') { // ENTER
                buffer[index] = '\0';
                shell_execute(buffer);
                index = 0;
                for(int i=0; i<256; i++) buffer[i] = 0;
                vga_print("\n> ");
            } 
            else if (c == '\b') { // BACKSPACE
                if (index > 0) {
                    index--;
                    buffer[index] = 0;
                    vga_backspace(); 
                }
            }
            else { // CARACTERES
                if (index < 255) {
                    vga_putchar(c);
                    buffer[index] = c;
                    index++;
                }
            }
            // Delay anti-repetição
            for(volatile int i=0; i<100000; i++) {}
        }
    }
}