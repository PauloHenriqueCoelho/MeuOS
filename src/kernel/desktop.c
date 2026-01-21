#include "../include/vga.h"
#include "../include/utils.h"

// Cores (Formato Hex: 0xAARRGGBB)
#define DESKTOP_COLOR 0xFF303080 // Azul Clássico
#define TASKBAR_COLOR 0xFFC0C0C0 // Cinza
#define TEXT_COLOR    0xFFFFFFFF // Branco

// Função Auxiliar para desenhar texto em posições absolutas
void desktop_print_label(int x, int y, char* str) {
    while(*str) {
        // Usa a nova função gfx_draw_char que aceita cor 32-bit
        gfx_draw_char(x, y, *str++, TEXT_COLOR);
        x += 8;
    }
}

void desktop_draw() {
    int w = get_screen_width();
    int h = get_screen_height();

    // 1. Limpa o fundo (Papel de Parede)
    gfx_fill_rect(0, 0, w, h, DESKTOP_COLOR);

    // 2. Barra de Tarefas (Rodapé)
    gfx_fill_rect(0, h - 30, w, 30, TASKBAR_COLOR);
    gfx_fill_rect(0, h - 30, w, 2, 0xFFFFFFFF); // Linha de luz (efeito 3D)

    // 3. Botão Iniciar (Simulado)
    gfx_fill_rect(4, h - 26, 60, 22, 0xFF808080); // Sombra do botão
    desktop_print_label(15, h - 18, "Start");

    // 4. Ícone: Terminal (Posição 20, 20)
    // Desenha um quadrado preto com borda
    gfx_fill_rect(20, 20, 32, 32, 0xFF000000); 
    gfx_draw_char(25, 25, '>', 0xFF00FF00); // Prompt verde
    desktop_print_label(18, 55, "Shell");

    // 5. Ícone: Calculadora (Posição 20, 80)
    // Desenha um quadrado branco com borda
    gfx_fill_rect(20, 80, 32, 32, 0xFFFFFFFF);
    gfx_draw_char(28, 88, '+', 0xFF000000); // Símbolo preto
    desktop_print_label(20, 115, "Calc");
}