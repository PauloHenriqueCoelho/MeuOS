#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void init_timer(uint32_t frequency);
void timer_callback();
void sleep(uint32_t ticks);
uint32_t get_tick();

#endif