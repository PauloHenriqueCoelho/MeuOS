#include "../include/window.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/programs.h"
#include "../include/shell.h" // Precisa acessar shell_draw

Window windows[MAX_WINDOWS];
int current_app_id = WIN_ID_NONE; // Define a variável global aqui

void wm_init() {
    for(int i=0; i<MAX_WINDOWS; i++) {
        windows[i].active = 0;
        windows[i].id = i;
    }
}
void wm_focus(int id) {
    if (id >= 0 && id < MAX_WINDOWS) {
        current_app_id = id;
    }
}

// Cria/Registra uma janela. Tenta achar slot livre.
int wm_create(int type, char* title, int x, int y, int w, int h, uint8_t color) {
    int id = -1;

    if (type == TYPE_SHELL) id = 0;
    else if (type == TYPE_CALC) id = 1;
    else {
        for(int i = 2; i < MAX_WINDOWS; i++) {
            if (!windows[i].active) { id = i; break; }
        }
    }

    if (id != -1) {
        windows[id].active = 1;
        windows[id].type = type;
        windows[id].x = x; windows[id].y = y;
        windows[id].w = w; windows[id].h = h;
        windows[id].color = color;
        strcpy(windows[id].title, title);
        
        // --- CORREÇÃO: Foco automático na criação ---
        current_app_id = id; 
    }
    return id;
}

void wm_close(int id) {
    if (id >= 0 && id < MAX_WINDOWS) windows[id].active = 0;
}

Window* wm_get(int id) {
    if (id >= 0 && id < MAX_WINDOWS && windows[id].active) return &windows[id];
    return 0;
}

// --- DESENHO CENTRALIZADO ---
void wm_draw_one(int id) {
    if (!windows[id].active) return;
    
    Window* w = &windows[id];

    // Tipo 1: Shell (Delegamos para o shell.c mas passando as coords)
    if (w->type == TYPE_SHELL) {
        shell_draw(); 
    }
    // Tipo 2: Calculadora
    else if (w->type == TYPE_CALC) {
        calculator_draw();
    }
    // Tipo 3: Janela de Texto Genérica (cat, msgbox)
    else if (w->type == TYPE_TEXT) {
        // 1. Desenha a Janela Base
        gfx_draw_window(w->title, w->x, w->y, w->w, w->h, w->color);
        
        // 2. Desenha o Conteúdo do Buffer
        // Ajuste de margem (X+10, Y+25)
        int cur_x = w->x + 10;
        int cur_y = w->y + 25;
        
        vga_set_color(15, w->color); // Branco sobre cor da janela
        
        // Simples renderizador de texto com quebra de linha manual
        int i = 0;
        int line_offset = 0;
        while(w->buffer[i]) {
            if (w->buffer[i] == '\n') {
                line_offset += 10; // Próxima linha
                cur_x = w->x + 10; // Reset X
            } else {
                gfx_draw_char(cur_x, cur_y + line_offset, w->buffer[i], 15);
                cur_x += 8;
                // Quebra automática se bater na borda
                if (cur_x > w->x + w->w - 10) {
                     line_offset += 10;
                     cur_x = w->x + 10;
                }
            }
            i++;
        }
    }
}

// --- Colisões (Idêntico ao anterior, mas iterando tudo) ---

int wm_check_title_collision(int mx, int my) {
    for(int i = MAX_WINDOWS - 1; i >= 0; i--) {
        if (!windows[i].active) continue;
        Window* w = &windows[i];
        // Note o "- 15" para deixar espaço para o botão [X]
        if (mx >= w->x && mx <= (w->x + w->w - 15) &&
            my >= w->y && my <= (w->y + 12)) return i;
    }
    return WIN_ID_NONE;
}

int wm_check_close_collision(int mx, int my) {
    for(int i=MAX_WINDOWS-1; i>=0; i--) {
        if (!windows[i].active) continue;
        Window* w = &windows[i];
        int bx = w->x + w->w - 10; int by = w->y + 2;
        if (mx >= bx && mx <= bx+8 && my >= by && my <= by+8) return i;
    }
    return WIN_ID_NONE;
}

int wm_check_body_collision(int mx, int my) {
    for(int i=MAX_WINDOWS-1; i>=0; i--) {
        if (!windows[i].active) continue;
        Window* w = &windows[i];
        if (mx >= w->x && mx <= w->x + w->w && my >= w->y && my <= w->y + w->h) return i;
    }
    return WIN_ID_NONE;
}