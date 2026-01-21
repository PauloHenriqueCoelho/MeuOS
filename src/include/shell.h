#ifndef SHELL_H
#define SHELL_H

#include "window.h" // Adicione este include para o tipo Window

void shell_init();
void shell_handle_key(char c); // Nova
void shell_execute(char* command);
void shell_draw(Window* w); // <--- Mude de void para Window* w
#endif