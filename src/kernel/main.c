#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/shell.h"
#include "../include/gdt.h" // GDT é essencial para estabilidade
#include "../include/idt.h"
#include "../include/keyboard.h"

void kernel_main() {
    // 1. VÍDEO (Fundo Azul/Ciano)
    video_init();
    
    // 2. MEMÓRIA (GDT)
    // Isso é obrigatório para as interrupções funcionarem sem crashar!
    init_gdt();

    // 3. INTERFACE (Desenha a Janela Preta)
    // Fazemos isso AGORA. Se o sistema travar depois, a janela já estará na tela.
    shell_init(); 
    
    // 4. HARDWARE (Interrupções e Teclado)
    init_idt();
    keyboard_init();
    
    // 5. Habilita a CPU
    __asm__ volatile("sti");

    // Loop Principal
    char buffer[256];
    int index = 0;
    for(int i=0; i<256; i++) buffer[i] = 0;
    
    // Posiciona o prompt dentro da janela (ajuste conforme necessário)
    vga_set_cursor(20, 50); 
    vga_print("> ");

    while(1) {
        char c = keyboard_get_key();

        if (c != 0) {
            if (c == '\n') { 
                buffer[index] = '\0';
                shell_execute(buffer);
                index = 0;
                for(int i=0; i<256; i++) buffer[i] = 0;
                
                // Prompt nova linha
                vga_print("\n> ");
            } 
            else if (c == '\b') {
               if (index > 0) { index--; buffer[index] = 0; vga_backspace(); }
            }
            else {
               if (index < 255) { vga_putchar(c); buffer[index] = c; index++; }
            }
        } else {
            // Economiza energia quando ocioso
            __asm__ volatile("hlt");
        }
    }
}