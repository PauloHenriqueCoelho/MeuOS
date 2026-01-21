#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define MAX_TASKS 10
#define STACK_SIZE 4096

typedef struct {
    uint32_t esp;          // Stack pointer salvo (onde a CPU parou)
    uint32_t stack_base;   // Endereço base da memória do stack (para o free)
    int active;
} task_t;

extern task_t tasks[MAX_TASKS];
extern int current_task;

void task_init();
int task_create(void* entry_point);
void scheduler();

#endif