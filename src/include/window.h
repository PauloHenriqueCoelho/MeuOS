#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 10
#define WIN_ID_SHELL 0  // <--- FALTAVA ISSO
#define WIN_ID_CALC  1

// Tipos de Janela
#define TYPE_SHELL 1
#define TYPE_CALC  3
#define TYPE_TEXT  2 // Janelas genéricas (msgbox, cat, help)

// IDs Fixos (Opcionais, mas úteis para apps do sistema)
#define WIN_ID_NONE  -1

extern int current_app_id;

typedef struct {
    int x, y, w, h;
    char label[16];
    int id; // ID que será retornado ao programa quando clicado
} Button;

typedef struct {
    int id;
    char title[32];
    int x, y, w, h;
    uint8_t color;
    int active;       // <-- Este é o nome que vamos usar!
    int type;
    char buffer[1024];
    Button buttons[10];
    int button_count;
} Window;

void wm_init();
void refresh_screen();
// Cria uma nova janela genérica e retorna o ID dela
int wm_create(int type, char* title, int x, int y, int w, int h, uint8_t color);
void wm_close(int id);
Window* wm_get(int id);
void wm_focus(int id);

// Função Mestra de Desenho (O WM decide como desenhar cada tipo)

// Colisões
int wm_check_title_collision(int mx, int my);
int wm_check_close_collision(int mx, int my);
int wm_check_body_collision(int mx, int my);
Window* wm_get(int id);
void wm_draw_one(Window* w);
int wm_wait_click(int win_id); // <-- ADICIONE ESTA LINHA

#endif