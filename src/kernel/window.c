#include "../include/window.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/programs.h"
#include "../include/mouse.h"
#include "../include/shell.h"
#include "../include/timer.h"
#include "../include/editor.h"
volatile int windows_changed = 0;   

Window windows[MAX_WINDOWS];
int current_app_id = WIN_ID_NONE;

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

void refresh_screen() {
    // 1. Fundo
    mouse_reset_background(); // Stub
    desktop_draw();
    
    // 2. Janelas de fundo
    for (int i = 0; i < MAX_WINDOWS; i++) {
        Window* w = wm_get(i);
        if (w && w->active && i != current_app_id) {
            wm_draw_one(w);
        }
    }

    // 3. Janela ativa
    Window* active_win = wm_get(current_app_id);
    if (active_win && active_win->active) {
        wm_draw_one(active_win);
    }
}

void wm_update_button(int win_id, int btn_id, char* new_text) {
    Window* w = wm_get(win_id);
    if (!w) return;

    for (int i = 0; i < w->button_count; i++) {
        if (w->buttons[i].id == btn_id) {
            strcpy(w->buttons[i].label, new_text);
            // CORREÇÃO: Redesenha AQUI para ver a mudança
            gfx_draw_button(w->buttons[i].label, w->x + w->buttons[i].x, w->y + w->buttons[i].y, w->buttons[i].w, w->buttons[i].h, 0xFFE0E0E0);
            return;
        }
    }
}

int wm_wait_click(int win_id) {
    // Loop de espera de evento
    while (1) {
        // Redesenha o mouse para não congelar o cursor nesta tarefa
        draw_mouse_cursor();

        if (mouse_get_status() & 1) { 
            int mx = mouse_get_x();
            int my = mouse_get_y();
            Window* w = wm_get(win_id);
            if (w) {
                // Checa colisão com botões
                for (int b = 0; b < w->button_count; b++) {
                    Button* btn = &w->buttons[b];
                    int bx = w->x + btn->x;
                    int by = w->y + btn->y;
                    
                    if (mx >= bx && mx <= bx + btn->w &&
                        my >= by && my <= by + btn->h) {
                        
                        // Efeito visual de clique (opcional)
                        gfx_draw_button(btn->label, bx, by, btn->w, btn->h, 0xFF808080); // Escurece
                        while (mouse_get_status() & 1); // Espera soltar
                        gfx_draw_button(btn->label, bx, by, btn->w, btn->h, 0xFFE0E0E0); // Restaura
                        
                        return btn->id;
                    }
                }
            }
            // Se clicou fora, espera soltar para não propagar
            while (mouse_get_status() & 1);
        }
        __asm__ volatile("hlt");
    }
    return WIN_ID_NONE;
}

int wm_create(int type, char* title, int x, int y, int w, int h, uint32_t color) {
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
        windows[id].x = x; 
        windows[id].y = y;
        windows[id].w = w; 
        windows[id].h = h;
        windows[id].color = color; 
        windows[id].button_count = 0;
        strcpy(windows[id].title, title);
        
        current_app_id = id;
        
        // CORREÇÃO CRÍTICA: Desenha a janela imediatamente ao criar!
        windows_changed = 1; // Avisa o Kernel para redesenhar no próximo ciclo
    }
    return id;
}

void wm_close(int id) {
    if (id >= 0 && id < MAX_WINDOWS) {
        windows[id].active = 0;
        refresh_screen(); // Limpa a tela se fechar
    }
}

Window* wm_get(int id) {
    if (id >= 0 && id < MAX_WINDOWS && windows[id].active) return &windows[id];
    return 0;
}

void wm_draw_one(Window* w) {
    if (!w || !w->active) return;

    gfx_draw_window(w->title, w->x, w->y, w->w, w->h, w->color);

    if (w->type == TYPE_SHELL) {
        shell_draw(w); 
    } 
    else if (w->type == TYPE_TEXT || w->type == TYPE_EDITOR) {
        int cur_x = w->x + 8;
        int cur_y = w->y + 25;
        char* ptr = w->buffer;
        while (*ptr) {
            if (*ptr == '\n') {
                cur_y += 10;
                cur_x = w->x + 8;
            } else {
                gfx_draw_char(cur_x, cur_y, *ptr, 0xFF000000);
                cur_x += 8;
            }
            ptr++;
        }
    }

    for (int i = 0; i < w->button_count; i++) {
        Button* b = &w->buttons[i];
        gfx_draw_button(b->label, w->x + b->x, w->y + b->y, b->w, b->h, 0xFFE0E0E0);
    }
}

// Colisões (Mantidas iguais)
int wm_check_title_collision(int mx, int my) {
    for(int i = MAX_WINDOWS - 1; i >= 0; i--) {
        if (!windows[i].active) continue;
        Window* w = &windows[i];
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