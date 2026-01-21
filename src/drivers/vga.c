#include "../include/vga.h"
#include "../include/utils.h"
#include "../include/font.h" // Certifique-se de que font.h existe com o array font8x8

// Variáveis do Framebuffer
static uint32_t* video_memory = 0;
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_pitch = 0;
static uint32_t screen_bpp = 0;

// Cursor do Console (Simulado em gráfico)
static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t text_color = COLOR_WHITE;

// --- Inicialização ---
void vga_init_from_multiboot(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    video_memory = (uint32_t*)(uint32_t)addr;
    screen_width = width;
    screen_height = height;
    screen_pitch = pitch;
    screen_bpp = bpp;
    vga_clear();
}

// --- Primitivas ---
void gfx_put_pixel(int x, int y, uint32_t color) {
    if (!video_memory) return;
    if (x < 0 || x >= (int)screen_width || y < 0 || y >= (int)screen_height) return;
    
    // Pitch geralmente é em bytes, e video_memory é ponteiro de 4 bytes (uint32_t)
    // offset = y * (pitch / 4) + x
    video_memory[y * (screen_pitch / 4) + x] = color;
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            gfx_put_pixel(x + j, y + i, color);
        }
    }
}

// --- Texto ---
void gfx_draw_char(int x, int y, char c, uint32_t color) {
    unsigned char uc = (unsigned char)c;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if (font8x8[uc][i] & (1 << j)) {
                // font8x8 geralmente é desenhada da direita pra esquerda nos bits ou vice-versa
                // Teste: Se ficar espelhado, mude para (0x80 >> j)
                gfx_put_pixel(x + j, y + i, color);
            }
        }
    }
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void vga_print_color(const char* str, uint32_t color) {
    while(*str) {
        char c = *str++;
        if (c == '\n') {
            cursor_x = 0;
            cursor_y += 10; // 8px altura + 2px margem
        } else {
            gfx_draw_char(cursor_x, cursor_y, c, color);
            cursor_x += 8;
        }
    }
}

void vga_print(const char* str) {
    vga_print_color(str, text_color);
}

void vga_clear() {
    // Limpa com Azul Escuro (Desktop Default)
    gfx_fill_rect(0, 0, screen_width, screen_height, 0xFF303080);
    cursor_x = 0;
    cursor_y = 0;
}

// --- GUI (Janelas e Botões em 32-bit) ---
void gfx_draw_window(char* title, int x, int y, int w, int h, uint32_t body_color) {
    // 1. Sombra (Transparência simulada ou preto sólido)
    gfx_fill_rect(x+4, y+4, w, h, 0xFF202020); 

    // 2. Corpo
    gfx_fill_rect(x, y, w, h, body_color);
    
    // 3. Barra de Título (Gradiente simples ou cor sólida)
    gfx_fill_rect(x, y, w, 20, 0xFF404040); // Cinza escuro
    
    // 4. Texto do Título
    int title_x = x + 6;
    int title_y = y + 6;
    for(int i=0; title[i] != 0; i++) {
        gfx_draw_char(title_x + (i*8), title_y, title[i], COLOR_WHITE);
    }

    // 5. Botão FECHAR [X]
    int btn_x = x + w - 18;
    int btn_y = y + 4;
    gfx_fill_rect(btn_x, btn_y, 14, 12, COLOR_RED);
    gfx_draw_char(btn_x + 3, btn_y + 2, 'X', COLOR_WHITE);
}

void gfx_draw_button(char* label, int x, int y, int w, int h, uint32_t color) {
    // Borda clara (topo/esquerda)
    gfx_fill_rect(x, y, w, h, 0xFFE0E0E0); // Borda clara
    // Borda escura (baixo/direita)
    gfx_fill_rect(x+1, y+1, w-1, h-1, 0xFF404040); 
    // Corpo
    gfx_fill_rect(x+1, y+1, w-2, h-2, color);

    // Texto Centralizado
    int len = strlen(label);
    int txt_x = x + (w/2) - (len*4);
    int txt_y = y + (h/2) - 4;
    
    for(int i=0; label[i]; i++) {
        gfx_draw_char(txt_x + (i*8), txt_y, label[i], COLOR_BLACK);
    }
}

void gfx_draw_cursor(int x, int y) {
    // Desenha seta do mouse (Branco com borda preta)
    // Simples bloco por enquanto
    gfx_fill_rect(x, y, 6, 6, COLOR_WHITE);
    gfx_fill_rect(x+1, y+1, 4, 4, COLOR_BLACK);
}

void gfx_clear_screen(uint32_t color) {
    if (!video_memory) return;
    
    // Otimização simples: preenche pixel a pixel
    // (Poderia ser otimizado com memset/rep stosd no futuro)
    for (uint32_t i = 0; i < screen_width * screen_height; i++) {
        video_memory[i] = color;
    }
    
    // Reseta cursor do console
    vga_set_cursor(0, 0);
}

int get_screen_width() { return screen_width; }
int get_screen_height() { return screen_height; }