#include "../include/window.h"
#include "../include/utils.h" // Para strcpy

Window windows[MAX_WINDOWS];

void wm_init() {
    for(int i=0; i<MAX_WINDOWS; i++) {
        windows[i].active = 0;
        windows[i].id = WIN_ID_NONE;
    }
}

// Abre (ou atualiza) uma janela
void wm_open(int id, char* title, int x, int y, int w, int h, uint8_t color) {
    if (id < 0 || id >= MAX_WINDOWS) return;
    
    // Se já estiver ativa, não reseta a posição (para não pular pro centro ao redesenhar)
    if (windows[id].active == 0) {
        windows[id].x = x;
        windows[id].y = y;
    }
    
    // Atualiza propriedades
    windows[id].id = id;
    windows[id].w = w;
    windows[id].h = h;
    windows[id].color = color;
    windows[id].active = 1;
    strcpy(windows[id].title, title);
}

void wm_close(int id) {
    if (id >= 0 && id < MAX_WINDOWS) {
        windows[id].active = 0;
    }
}

Window* wm_get(int id) {
    if (id >= 0 && id < MAX_WINDOWS) {
        return &windows[id];
    }
    return 0; // Null
}

// --- VERIFICAÇÕES DE COLISÃO GENÉRICAS ---

int wm_check_title_collision(int mx, int my) {
    // Percorre todas as janelas ativas
    for(int i=0; i<MAX_WINDOWS; i++) {
        if (!windows[i].active) continue;

        Window* w = &windows[i];
        // Barra de título tem 12px de altura
        // Ignora os últimos 15px (botão fechar)
        if (mx >= w->x && mx <= (w->x + w->w - 15) &&
            my >= w->y && my <= (w->y + 12)) {
            return w->id;
        }
    }
    return WIN_ID_NONE;
}

int wm_check_close_collision(int mx, int my) {
    for(int i=0; i<MAX_WINDOWS; i++) {
        if (!windows[i].active) continue;

        Window* w = &windows[i];
        // Botão Fechar (x + w - 10, y + 2, 8x8)
        int btn_x = w->x + w->w - 10;
        int btn_y = w->y + 2;
        
        if (mx >= btn_x && mx <= btn_x + 8 &&
            my >= btn_y && my <= btn_y + 8) {
            return w->id;
        }
    }
    return WIN_ID_NONE;
}

int wm_check_body_collision(int mx, int my) {
    for(int i=0; i<MAX_WINDOWS; i++) {
        if (!windows[i].active) continue;
        Window* w = &windows[i];
        
        if (mx >= w->x && mx <= w->x + w->w &&
            my >= w->y && my <= w->y + w->h) {
            return w->id;
        }
    }
    return WIN_ID_NONE;
}