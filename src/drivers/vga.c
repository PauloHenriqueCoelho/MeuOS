#include "../include/io.h"
#include "../include/utils.h"
#include "../include/font.h" 
#include "../include/vga.h" // Importante ter o header

// Memória de vídeo no modo 13h (Linear)
uint8_t* VGA_MEMORY = (uint8_t*)0xA0000;

#define VGA_WIDTH 320
#define VGA_HEIGHT 200

// Cursor e Cores
int cursor_x = 0;
int cursor_y = 0;
uint8_t current_color = 15; // Branco
uint8_t bg_color = 1;       // Azul Escuro (para diferenciar do preto de erro)

// --- CONFIGURAÇÃO DE HARDWARE (MODO 13h) ---
void vga_set_mode_13h() {
    // 1. MISC (Porta 0x3C2)
    outb(0x3C2, 0x63);

    // 2. SEQUENCER (Porta 0x3C4)
    outb(0x3C4, 0); outb(0x3C5, 0x03); // Reset
    outb(0x3C4, 1); outb(0x3C5, 0x01); // Clocking Mode
    outb(0x3C4, 2); outb(0x3C5, 0x0F); // Map Mask
    outb(0x3C4, 3); outb(0x3C5, 0x00); // Char Map Select
    outb(0x3C4, 4); outb(0x3C5, 0x0E); // Memory Mode
    outb(0x3C4, 0); outb(0x3C5, 0x03); // Run (Fim do Reset)

    // 3. CRTC (Porta 0x3D4) - Controlador de Tubo de Raios Catódicos
    // Destrava registradores (CRTC Register 11 bit 7)
    outb(0x3D4, 0x11);
    uint8_t v = inb(0x3D5);
    outb(0x3D5, v & 0x7F); 

    uint8_t crtc_vals[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF 
    };
    for(int i=0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc_vals[i]);
    }

    // 4. GRAPHICS CONTROLLER (Porta 0x3CE)
    outb(0x3CE, 0); outb(0x3CF, 0x00); // Set/Reset
    outb(0x3CE, 1); outb(0x3CF, 0x00); // Enable Set/Reset
    outb(0x3CE, 2); outb(0x3CF, 0x00); // Color Compare
    outb(0x3CE, 3); outb(0x3CF, 0x00); // Data Rotate
    outb(0x3CE, 4); outb(0x3CF, 0x00); // Read Map Select
    outb(0x3CE, 5); outb(0x3CF, 0x40); // Mode (Bit 6 = 256 color mode)
    outb(0x3CE, 6); outb(0x3CF, 0x05); // Misc
    outb(0x3CE, 7); outb(0x3CF, 0x0F); // Color Don't Care
    outb(0x3CE, 8); outb(0x3CF, 0xFF); // Bit Mask

    // 5. ATTRIBUTE CONTROLLER (Porta 0x3C0)
    uint8_t attr_vals[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };
    for(int i=0; i < 21; i++) {
        inb(0x3DA); // Reseta o flip-flop lendo o Status Register
        outb(0x3C0, i);
        outb(0x3C0, attr_vals[i]);
    }
    inb(0x3DA);
    outb(0x3C0, 0x20); // Reabilita acesso à paleta
}

// --- PRIMITIVAS DE DESENHO ---
void gfx_put_pixel(int x, int y, uint8_t color) {
    // Escreve direto na memória (mais rápido e seguro agora que a GDT funciona)
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        VGA_MEMORY[y * VGA_WIDTH + x] = color;
    }
}

void gfx_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, color);
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint8_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc > 127) return; 
    
    // Desenha pixel a pixel baseado na font.h
    for (int i = 0; i < 8; i++) {
        uint8_t row = font8x8[uc][i];
        for (int j = 0; j < 8; j++) {
            if (row & (0x80 >> j)) {
                gfx_put_pixel(x + j, y + i, color);
            }
        }
    }
}

// --- FUNÇÕES DE TEXTO / API ---

void vga_clear() {
    gfx_fill_rect(0, 0, VGA_WIDTH, VGA_HEIGHT, bg_color);
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 18; 
        cursor_y += 8;
    } else if (c == '\b') {
        if (cursor_x >= 8) {
            cursor_x -= 8;
            gfx_fill_rect(cursor_x, cursor_y, 8, 8, bg_color); // Apaga com a cor de fundo
        }
    } else {
        gfx_draw_char(cursor_x, cursor_y, c, current_color);
        cursor_x += 8;
    }
}

void vga_backspace() {
    vga_putchar('\b');
}

void vga_print(const char* str) {
    while(*str) {
        vga_putchar(*str++);
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = fg;
    bg_color = bg; // Atualiza a cor de fundo também
}

// --- JANELAS ---
void gfx_draw_window(char* title, int x, int y, int w, int h, uint8_t body_color) {
    // 1. Sombra (Preto)
    gfx_fill_rect(x+2, y+2, w, h, 0); 

    // 2. Corpo
    gfx_fill_rect(x, y, w, h, body_color);
    
    // 3. Barra de Título
    gfx_fill_rect(x, y, w, 12, 8); // Cinza Escuro (Altura aumentada para 12px para caber o botão)
    
    // 4. Texto do Título
    int title_len = strlen(title);
    int title_x = x + 4; // Alinhado à esquerda fica mais bonito com o botão na direita
    for(int i=0; i < title_len; i++) {
        gfx_draw_char(title_x + (i*8), y + 2, title[i], 15);
    }

    // 5. Botão FECHAR [X] (A Mágica acontece aqui)
    // Posição: Canto direito da barra (x + largura - 10 pixels)
    int btn_x = x + w - 10;
    int btn_y = y + 2;
    
    gfx_fill_rect(btn_x, btn_y, 8, 8, 4); // Fundo Vermelho (Cor 4)
    gfx_draw_char(btn_x, btn_y, 'X', 15); // Letra X Branca (Cor 15)
}

void video_init() {
    vga_set_mode_13h();
    vga_clear();
}

void gfx_draw_button(char* label, int x, int y, int w, int h, uint8_t color) {
    // 1. Fundo principal do botão
    gfx_fill_rect(x, y, w, h, color);

    // 2. Efeito 3D (Bordas)
    // Borda superior e esquerda branca (luz)
    gfx_fill_rect(x, y, w, 1, 15); 
    gfx_fill_rect(x, y, 1, h, 15);
    
    // Borda inferior e direita preta (sombra)
    gfx_fill_rect(x, y + h - 1, w, 1, 0);
    gfx_fill_rect(x + w - 1, y, 1, h, 0);

    // 3. Desenhar o Texto (Centralizado)
    // Se você tiver a função vga_print_at ou similar:
    int label_len = strlen(label);
    int text_x = x + (w / 2) - (label_len * 4); // Ajuste fino para 8px por char
    int text_y = y + (h / 2) - 4;
    
    // Use sua função de desenho de caracteres aqui
    for(int i = 0; label[i] != '\0'; i++) {
        gfx_draw_char(text_x + (i * 8), text_y, label[i], 0); // Texto preto
    }
}

void vga_print_at(int x, int y, char* str, uint8_t color) {
    vga_set_cursor(x, y);
    // Assumindo que sua vga_print aceita cor ou usa uma global
    vga_print(str); 
}

// No arquivo src/drivers/vga.c
void gfx_draw_cursor(int x, int y) {
    // Desenha uma seta branca simples com borda preta
    // Usando 15 para branco e 0 para preto
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j <= i; j++) {
            vga_plot_pixel(x + j, y + i, 15);
        }
    }
    // Opcional: desenha uma linha preta na borda da seta para visibilidade
    for (int i = 0; i < 8; i++) {
        vga_plot_pixel(x + i, y + i, 0);
        vga_plot_pixel(x, y + i, 0);
    }
}

void vga_put_pixel(int x, int y, uint8_t color) {
    uint8_t* screen = (uint8_t*)0xA0000; // Endereço padrão do Modo 13h
    screen[y * 320 + x] = color;
}
// No arquivo src/drivers/vga.c

void vga_plot_pixel(int x, int y, uint8_t color) {
    // Proteção básica para não escrever fora da memória VGA (320x200)
    if (x < 0 || x >= 320 || y < 0 || y >= 200) return;

    // Endereço base da memória de vídeo em modo 13h
    uint8_t* vga_mem = (uint8_t*)0xA0000;
    
    // A fórmula mágica: cada linha tem 320 pixels
    vga_mem[y * 320 + x] = color;
}