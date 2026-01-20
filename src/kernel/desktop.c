#include "../include/vga.h"
#include "../include/programs.h"
#include "../include/window.h" 
#include "../include/utils.h"

// Variáveis da Calc (Lógica apenas)
int calc_accumulator = 0;
int calc_current_val = 0;
int calc_reset_screen = 0;

void draw_icon(int x, int y, uint8_t color, char letter, char* label) {
    gfx_fill_rect(x, y, 32, 32, color);
    gfx_fill_rect(x, y, 32, 2, 15);
    gfx_fill_rect(x, y, 2, 32, 15);
    gfx_fill_rect(x+30, y, 2, 32, 8);
    gfx_fill_rect(x, y+30, 32, 2, 8);
    gfx_draw_char(x + 12, y + 12, letter, 15);
    int len = 0; while(label[len]) len++;
    vga_set_cursor(x + (16 - (len * 4)), y + 36);
    vga_print(label);
}

void desktop_draw() {
    gfx_fill_rect(0, 0, 320, 200, 3);
    gfx_fill_rect(0, 185, 320, 15, 7);
    gfx_fill_rect(0, 185, 320, 1, 15);
    
    gfx_fill_rect(2, 187, 40, 11, 8);
    vga_set_cursor(6, 189);
    vga_print("MeuOS");

    draw_icon(20, 20, 0, '>', "Shell");
    draw_icon(20, 80, 4, '+', "Calc");
}

// --- DESENHO DOS PROGRAMAS ---

void calc_update_screen(int num) {
    Window* w = wm_get(WIN_ID_CALC);
    if (!w) return;

    int screen_x = w->x + 10;
    int screen_y = w->y + 20;

    gfx_fill_rect(screen_x, screen_y, 100, 20, 15);
    
    char buffer[16];
    int i = 0, n = num;
    if (n == 0) { buffer[0]='0'; buffer[1]='\0'; }
    else {
        while (n > 0) { buffer[i++] = (n % 10) + '0'; n /= 10; }
        buffer[i] = '\0';
        for(int j=0; j<i/2; j++) { char t=buffer[j]; buffer[j]=buffer[i-1-j]; buffer[i-1-j]=t; }
    }
    
    int len = 0; while(buffer[len]) len++;
    vga_set_cursor((screen_x + 90) - (len*8), screen_y + 6);
    vga_set_color(0, 15);
    vga_print(buffer);
}

void calculator_draw() {
    // 1. Pega dados da janela
    Window* w = wm_get(WIN_ID_CALC);
    if (!w) return;
    
    // 2. Desenha a janela
    gfx_draw_window(w->title, w->x, w->y, w->w, w->h, w->color);
    
    calc_update_screen(calc_current_val);
    
    char labels[12] = {'7','8','9', '4','5','6', '1','2','3', 'C','0','='};
    int start_x = w->x + 10;
    int start_y = w->y + 50;
    
    // --- CORREÇÃO: Usamos 'bw' (button width) para não conflitar com 'w' (janela)
    int bw = 30; 
    int bh = 20; 
    int padding = 5;

    for(int i=0; i<12; i++) {
        int col = i % 3; int row = i / 3;
        int bx = start_x + (col * (bw + padding));
        int by = start_y + (row * (bh + padding));
        
        uint8_t color = 8;
        if (labels[i] == 'C') color = 4;
        if (labels[i] == '=') color = 2;
        
        gfx_fill_rect(bx, by, bw, bh, color);
        gfx_fill_rect(bx, by, bw, 1, 15);
        gfx_fill_rect(bx, by, 1, bh, 15);
        gfx_draw_char(bx + 11, by + 6, labels[i], 15);
    }
}

void calculator_click(int mx, int my) {
    Window* w = wm_get(WIN_ID_CALC);
    if (!w) return;
    
    char labels[12] = {'7','8','9', '4','5','6', '1','2','3', 'C','0','='};
    int start_x = w->x + 10;
    int start_y = w->y + 50;
    
    // --- CORREÇÃO: 'bw' aqui também
    int bw = 30; 
    int bh = 20; 
    int padding = 5;

    for(int i=0; i<12; i++) {
        int col = i % 3; int row = i / 3;
        int bx = start_x + (col * (bw + padding));
        int by = start_y + (row * (bh + padding));

        if (mx >= bx && mx <= bx+bw && my >= by && my <= by+bh) {
            char key = labels[i];

            if (key >= '0' && key <= '9') {
                int digit = key - '0';
                if (calc_reset_screen) { calc_current_val = 0; calc_reset_screen = 0; }
                if (calc_current_val < 10000) calc_current_val = (calc_current_val * 10) + digit;
                calc_update_screen(calc_current_val);
            }
            else if (key == 'C') {
                calc_current_val = 0; calc_accumulator = 0; calc_update_screen(0);
            }
            else if (key == '=') {
                calc_accumulator += calc_current_val; calc_current_val = calc_accumulator;
                calc_update_screen(calc_current_val); calc_reset_screen = 1; calc_current_val = 0;
            }
            return; 
        }
    }
}