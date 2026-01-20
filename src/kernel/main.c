#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/shell.h"

// --- Mapa de Teclado (US Layout) ---
char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// --- INICIALIZAÇÃO SEGURA DO TECLADO ---
void keyboard_init() {
    // 1. Ativa a porta do teclado na placa-mãe
    outb(0x64, 0xAE);
    
    // Delay simples para o hardware respirar
    for(volatile int i=0; i<1000; i++) {}

    // 2. Manda o comando "LIGAR" (0xF4) para o teclado
    // Espera o buffer de comando ficar livre antes de mandar
    while((inb(0x64) & 0x2)) {} 
    outb(0x60, 0xF4); 

    // 3. O teclado manda um "OK" (ACK 0xFA). Vamos ler para limpar.
    // Loop com limite para não travar o PC se falhar
    for(int i=0; i<10000; i++) {
        if(inb(0x64) & 0x1) {
            inb(0x60); // Lê e descarta o ACK
            break;
        }
    }
}

char keyboard_read_polling() {
    unsigned char status;
    unsigned char scancode;

    status = inb(0x64); // Pergunta: Tem dados?

    if (status & 0x01) {
        scancode = inb(0x60); // Lê o dado
        
        // Verifica se é tecla pressionada (bit 7 = 0)
        if (!(scancode & 0x80)) {
            if (scancode < 128) {
                return kbd_US[scancode];
            }
        }
    }
    return 0;
}

void kernel_main() {
    vga_clear();
    
    // NÃO carregamos GDT/IDT aqui. Vamos manter simples para não travar.
    
    vga_print("=== MyOS Shell (Modo Polling) ===\n");
    vga_print("Inicializando teclado...\n");
    
    keyboard_init(); // Ativa o hardware
    
    vga_print("Teclado Ativo. Digite 'ajuda':\n");
    vga_print("> ");

    char buffer[256];
    int index = 0;
    
    // Limpa buffer
    for(int i=0; i<256; i++) buffer[i] = 0;

    while(1) {
        char c = keyboard_read_polling();

        if (c != 0) {
            // ENTER
            if (c == '\n') {
                buffer[index] = '\0';
                shell_execute(buffer);
                
                index = 0;
                for(int i=0; i<256; i++) buffer[i] = 0;
                vga_print("\n> ");
            } 
            // BACKSPACE
            else if (c == '\b') {
                if (index > 0) {
                    index--;
                    buffer[index] = 0;
                    vga_backspace(); 
                }
            }
            // CARACTERES
            else {
                if (index < 255) {
                    vga_putchar(c);
                    buffer[index] = c;
                    index++;
                }
            }
            
            // Delay para evitar repetição
            for(volatile int i=0; i<100000; i++) {}
        }
    }
}