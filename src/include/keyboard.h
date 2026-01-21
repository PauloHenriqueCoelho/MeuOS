#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "idt.h" // <--- Adicione

void keyboard_init();
// Se keyboard_handler_isr estiver declarado aqui, mude para:
void keyboard_handler_isr(registers_t* r);

char keyboard_get_key();

#endif