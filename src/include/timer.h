#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "idt.h" // <--- Importante: Define o que é 'registers_t'

void init_timer(uint32_t frequency);

// CORREÇÃO: Atualizado para aceitar ponteiro
void timer_callback(registers_t* regs);

void sleep(uint32_t ticks);
uint32_t get_tick();

#endif