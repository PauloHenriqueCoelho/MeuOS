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
    mouse_reset_background();
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

    //draw_clock();
}


// Atualiza o texto de um botão específico numa janela
void wm_update_button(int win_id, int btn_id, char* new_text) {
    Window* w = wm_get(win_id);
    if (!w) return;

    for (int i = 0; i < w->button_count; i++) {
        if (w->buttons[i].id == btn_id) {
            // Copia o novo texto para o botão (Cuidado com overflow aqui no futuro!)
            // Assumindo que você tem strcpy no utils.h
            strcpy(w->buttons[i].label, new_text);
            wm_draw_one(w); // Força o redesenho IMEDIATO da janela!
            return;
        }
    }
}

// No arquivo src/kernel/window.c
int wm_wait_click(int win_id) {
    while (1) {
        // Redesenha a tela e o mouse a cada iteração para manter a responsividade
        draw_mouse_cursor();

        if (mouse_get_status() & 1) {  // Botão esquerdo pressionado
            int mx = mouse_get_x();
            int my = mouse_get_y();
            Window* w = wm_get(win_id);
            if (w) {
                // Verifica colisão com botões da janela
                for (int b = 0; b < w->button_count; b++) {
                    Button* btn = &w->buttons[b];
                    int bx = w->x + btn->x;
                    int by = w->y + btn->y;
                    if (mx >= bx && mx <= bx + btn->w &&
                        my >= by && my <= by + btn->h) {
                        while (mouse_get_status() & 1);  // Debounce
                        return btn->id;
                    }
                }
            }
            while (mouse_get_status() & 1);  // Debounce se não for botão
        }
        __asm__ volatile("hlt");  // Espera próxima interrupção
    }
    return WIN_ID_NONE;  // Nunca chega aqui, mas para complacência
}
// Cria/Registra uma janela. Tenta achar slot livre.
int wm_create(int type, char* title, int x, int y, int w, int h, uint32_t color) {
    int id = -1;

    // Lógica de slot fixo para Shell(0) e Calc(1)
    if (type == TYPE_SHELL) id = 0;
    else if (type == TYPE_CALC) id = 1;
    else {
        // Busca livre
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
        windows[id].color = color; // Agora cabe o 0xFFC0C0C0
        windows[id].button_count = 0;
        strcpy(windows[id].title, title);
        
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

    // Chama a função gráfica nova (32-bit)
    gfx_draw_window(w->title, w->x, w->y, w->w, w->h, w->color);

    // Conteúdo
    if (w->type == TYPE_SHELL) {
        shell_draw(w); 
    } 
    else if (w->type == TYPE_TEXT) {
        // Texto Simples
        int cur_x = w->x + 8;
        int cur_y = w->y + 25;
        char* ptr = w->buffer;
        while (*ptr) {
            if (*ptr == '\n') {
                cur_y += 10;
                cur_x = w->x + 8;
            } else {
                gfx_draw_char(cur_x, cur_y, *ptr, 0xFF000000); // Texto Preto
                cur_x += 8;
            }
            ptr++;
        }
    }

    // Botões
    for (int i = 0; i < w->button_count; i++) {
        Button* b = &w->buttons[i];
        // Botão Cinza Claro
        gfx_draw_button(b->label, w->x + b->x, w->y + b->y, b->w, b->h, 0xFFE0E0E0);
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