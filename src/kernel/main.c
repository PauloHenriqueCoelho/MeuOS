#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/shell.h"
#include "../include/gdt.h" // Se tiver GDT
#include "../include/idt.h" // <--- Importante
#include "../include/keyboard.h"

// O Linker procura por kernel_main
void kernel_main() {
    video_init();    
    // 1. Inicializa tabelas
    // init_gdt(); // Se você tiver o arquivo gdt.c funcionando, descomente
    init_idt();   // Configura o PIC e a Tabela de Interrupções
    
    // 2. Inicializa Hardware
    keyboard_init();
    shell_init();

    // 3. Habilita INTERRUPÇÕES (A Chave Mestra)
    // A partir daqui, se apertar tecla, o 'isr1' no assembly acorda
    __asm__ volatile("sti");

    vga_print("=== MyOS v0.6 (Interrupt Driven) ===\n");
    vga_print("CPU Ociosa (HLT) ativada.\n");
    vga_print("> ");

    char buffer[256];
    int index = 0;
    for(int i=0; i<256; i++) buffer[i] = 0;

    while(1) {
        // Agora isso pega do buffer, não da porta direta!
        char c = keyboard_get_key();

        if (c != 0) {
            if (c == '\n') { 
                buffer[index] = '\0';
                shell_execute(buffer);
                index = 0;
                for(int i=0; i<256; i++) buffer[i] = 0;
                vga_print("\n> ");
            } 
            else if (c == '\b') {
               if (index > 0) { index--; buffer[index] = 0; vga_backspace(); }
            }
            else {
               if (index < 255) { vga_putchar(c); buffer[index] = c; index++; }
            }
        } else {
            // Se não tem tecla, dorme até a próxima interrupção
            __asm__ volatile("hlt");
        }
    }
}