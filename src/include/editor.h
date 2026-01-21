// MeuOS/src/include/editor.h
#ifndef EDITOR_H
#define EDITOR_H

#include "window.h"

#define TYPE_EDITOR 3

void editor_open(char* filename);
void editor_handle_key(char c);
void editor_save(); // Adicione esta linha

#endif