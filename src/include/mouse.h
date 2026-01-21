#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>
#include "idt.h" // <--- Adicione se não tiver


void mouse_init();
int mouse_get_x();
int mouse_get_y();
int mouse_get_status(); // Retorna botões clicados
void mouse_reset_background();
void draw_mouse_cursor(); // <-- ADICIONE ESTA LINHA
void mouse_handler_isr(registers_t* r); 

#endif