#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 10

// IDs dos Programas
#define WIN_ID_NONE  -1
#define WIN_ID_SHELL 0
#define WIN_ID_CALC  1

// Estrutura de uma Janela
typedef struct {
    int id;           // ID único (0=Shell, 1=Calc)
    int x, y;         // Posição
    int w, h;         // Tamanho
    char title[16];   // Título da barra
    uint8_t color;    // Cor do fundo
    int active;       // 1 = Aberta, 0 = Fechada
    int z_index;      // (Futuro) Para saber quem está na frente
} Window;

// Funções do Gerenciador
void wm_init();
void wm_open(int id, char* title, int x, int y, int w, int h, uint8_t color);
void wm_close(int id);
Window* wm_get(int id);

// Retorna o ID da janela se o mouse estiver na BARRA DE TÍTULO
int wm_check_title_collision(int mouse_x, int mouse_y);

// Retorna o ID da janela se o mouse estiver no BOTÃO FECHAR
int wm_check_close_collision(int mouse_x, int mouse_y);

// Retorna o ID da janela se o mouse estiver no CORPO (útil para cliques internos)
int wm_check_body_collision(int mouse_x, int mouse_y);

#endif