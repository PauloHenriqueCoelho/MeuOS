#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 10
#define WIN_ID_NONE -1

// Tipos de Janela
#define TYPE_SHELL 0
#define TYPE_CALC  1
#define TYPE_TEXT  2

typedef struct {
    int id;
    int x, y, w, h;
    char label[16];
} Button;

typedef struct {
    int id;
    int type;
    int active;
    char title[32];
    int x, y, w, h;
    uint32_t color;  // <--- MUDOU DE uint8_t PARA uint32_t
    
    Button buttons[10];
    int button_count;
    
    // Buffer para janelas de texto
    char buffer[256];
} Window;

// IDs Fixos das Aplicações Principais
#define WIN_ID_SHELL 0  // <--- ADICIONADO: O Shell sempre ocupa o slot 0
#define WIN_ID_CALC  1

void wm_init();
int wm_create(int type, char* title, int x, int y, int w, int h, uint32_t color); // <--- uint32_t AQUI
void wm_close(int id);
Window* wm_get(int id);
void wm_draw_one(Window* w);
void wm_focus(int id);
void wm_update_button(int win_id, int btn_id, char* new_text);
void refresh_screen(); // O compilador reclamava disso
void desktop_draw();   // Necessário para o refresh_screen desenhar o fundo

// Colisões
int wm_check_title_collision(int mx, int my);
int wm_check_close_collision(int mx, int my);
int wm_check_body_collision(int mx, int my);
int wm_wait_click(int win_id);

extern int current_app_id;

#endif