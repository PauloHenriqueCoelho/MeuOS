#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/io.h" // Para o comando reboot

void shell_execute(char* input) {
    // Se o usuário apertou Enter sem digitar nada, ignorar
    if (strlen(input) == 0) return;

    vga_print("\n"); // Pula linha após o comando digitado

    // --- COMANDO: AJUDA ---
    if (strcmp(input, "ajuda") == 0) {
        vga_print("Comandos disponiveis:\n");
        vga_print("  ajuda    - Mostra esta lista\n");
        vga_print("  limpar   - Limpa a tela\n");
        vga_print("  reboot   - Reinicia o computador (Hardware Reset)\n");
        vga_print("  sobre    - Informacoes do sistema\n");
    } 
    // --- COMANDO: LIMPAR ---
    else if (strcmp(input, "limpar") == 0) {
        vga_clear();
        return; // Retorna para não imprimir nova linha extra
    }
    // --- COMANDO: SOBRE ---
    else if (strcmp(input, "sobre") == 0) {
        vga_print("MyOS v0.1 - Kernel Modular em C\n");
        vga_print("Desenvolvido do zero.\n");
    }
    // --- COMANDO: REBOOT (Magia de Hardware) ---
    else if (strcmp(input, "reboot") == 0) {
        vga_print("Reiniciando sistema...\n");
        // Envia comando de reset para o Controlador de Teclado (8042)
        // O bit 0 da porta 0x64 controla o reset da CPU.
        uint8_t temp = inb(0x64);
        while (temp & 0x02) temp = inb(0x64); // Espera buffer limpar
        outb(0x64, 0xFE); // Envia pulso de reset
        
        // Se falhar, trava
        asm volatile("hlt");
    }
    // --- COMANDO DESCONHECIDO ---
    else {
        vga_print("Comando desconhecido: ");
        vga_print(input);
        vga_print("\n");
    }
}