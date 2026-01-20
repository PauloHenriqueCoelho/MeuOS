#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>

void mouse_init();
int mouse_get_x();
int mouse_get_y();
int mouse_get_status(); // Retorna botões clicados
void mouse_reset_background();

#endif