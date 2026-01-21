#include "../include/window.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/programs.h"
#include "../include/mouse.h" // <--- IMPORTANTE: Adicione este include!
#include "../include/shell.h" // Precisa acessar shell_draw
#include "../include/timer.h" // <--- ADICIONE ESTA LINHA
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

// No arquivo src/kernel/window.c

void refresh_screen() {
    // 1. Desenha o fundo (Desktop, ícones e papel de parede)
    desktop_draw();
    
    // 2. Desenha as janelas de fundo
    for (int i = 0; i < MAX_WINDOWS; i++) {
        Window* w = wm_get(i);
        if (w && w->active && i != current_app_id) {
            wm_draw_one(w);
        }
    }

    // 3. Desenha a janela com foco (sempre por cima das outras)
    Window* active_win = wm_get(current_app_id);
    if (active_win && active_win->active) {
        wm_draw_one(active_win);
    }

    // 4. DESENHA O MOUSE (A camada mais alta de todas)
    // Usamos as funções do seu driver de mouse
    int mx = mouse_get_x();
    int my = mouse_get_y();
    
    // Aqui você usa sua função de desenho de cursor ou uma simples:
    gfx_draw_cursor(mx, my); 

}

int wm_wait_click(int win_id) {
    Window* w = wm_get(win_id);
    if (!w) return -1;

    uint32_t last_sync = 0;

    while (1) {
        // LIMITADOR DE FPS: Só redesenha se passou tempo suficiente (aprox. 16ms)
        uint32_t current_tick = get_tick();
        if (current_tick > last_sync) {
            refresh_screen();
            last_sync = current_tick + 1; // Ajuste conforme a velocidade do seu timer
        }

        if (mouse_get_status() & 1) {
            int mx = mouse_get_x();
            int my = mouse_get_y();
            
            for (int i = 0; i < w->button_count; i++) {
                Button* b = &w->buttons[i];
                if (mx >= (w->x + b->x) && mx <= (w->x + b->x + b->w) &&
                    my >= (w->y + b->y) && my <= (w->y + b->y + b->h)) {
                    
                    while(mouse_get_status() & 1) refresh_screen();
                    return b->id;
                }
            }
        }
        __asm__ volatile("pause");
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
void wm_draw_one(Window* w) {
    if (!w || !w->active) return;

    // 1. Desenha a moldura e o fundo da janela
    gfx_draw_window(w->title, w->x, w->y, w->w, w->h, w->color);

    // 2. DESENHA O CONTEÚDO (O que estava faltando!)
    if (w->type == TYPE_SHELL) {
        // Passamos a janela para o shell saber onde desenhar o texto
        shell_draw(w); 
    } 
    else if (w->type == TYPE_TEXT) {
        // CORREÇÃO PARA MULTILINHAS:
        int cur_x = w->x + 8;
        int cur_y = w->y + 20;
        char* ptr = w->buffer;
        
        while (*ptr) {
            if (*ptr == '\n') {
                cur_y += 10;     // Pula linha
                cur_x = w->x + 8; // Volta para o X da JANELA, não da tela!
            } else {
                gfx_draw_char(cur_x, cur_y, *ptr, 0); // Desenha caractere
                cur_x += 8;
            }
            ptr++;
        }
    }
    // 3. Desenha os botões por cima de tudo
    for (int i = 0; i < w->button_count; i++) {
        Button* b = &w->buttons[i];
        gfx_draw_button(b->label, w->x + b->x, w->y + b->y, b->w, b->h, 7);
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