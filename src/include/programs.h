#ifndef PROGRAMS_H
#define PROGRAMS_H

// Estados do Sistema
#define STATE_DESKTOP 0
#define STATE_SHELL   1
#define STATE_CALC    2

// --- VARIÁVEIS DE POSIÇÃO DAS JANELAS (NOVO) ---
extern int win_shell_x, win_shell_y;
extern int win_calc_x, win_calc_y;

// Variáveis da Calculadora
extern int calc_num1;
extern int calc_op; 
extern int calc_result;
extern int calc_new_entry;

// Funções
void desktop_draw();
void calculator_draw();
void calculator_click(int mx, int my);

#endif