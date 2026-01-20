#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 10
#define WIN_ID_SHELL 0  // <--- FALTAVA ISSO
#define WIN_ID_CALC  1

// Tipos de Janela
#define TYPE_SHELL 0
#define TYPE_CALC  1
#define TYPE_TEXT  2 // Janelas genéricas (msgbox, cat, help)

// IDs Fixos (Opcionais, mas úteis para apps do sistema)
#define WIN_ID_NONE  -1

extern int current_app_id;

typedef struct {
    int id;
    int type;           // TIPO DA JANELA
    int x, y, w, h;
    char title[16];
    uint8_t color;
    int active;
    
    // Buffer de conteúdo (Para janelas de texto genéricas)
    char buffer[1024]; 
} Window;

void wm_init();
// Cria uma nova janela genérica e retorna o ID dela
int wm_create(int type, char* title, int x, int y, int w, int h, uint8_t color);
void wm_close(int id);
Window* wm_get(int id);
void wm_focus(int id);

// Função Mestra de Desenho (O WM decide como desenhar cada tipo)
void wm_draw_one(int id);

// Colisões
int wm_check_title_collision(int mx, int my);
int wm_check_close_collision(int mx, int my);
int wm_check_body_collision(int mx, int my);

#endif