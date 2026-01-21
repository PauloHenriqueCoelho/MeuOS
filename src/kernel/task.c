#include "../include/task.h"
#include "../include/memory.h"
#include "../include/utils.h"

task_t tasks[MAX_TASKS];
int current_task = 0;
int task_count = 1; // Tarefa 0 é o Kernel/Main

void task_init() {
    for(int i=0; i<MAX_TASKS; i++) tasks[i].active = 0;
    tasks[0].active = 1; // O código que está rodando agora vira a Task 0
}

int task_create(void* entry_point) {
    int id = -1;
    for(int i = 1; i < MAX_TASKS; i++) {
        if(!tasks[i].active) { id = i; break; }
    }
    if (id == -1) return -1;

    // Aloca 4KB para a pilha do programa usando o seu PMM
    tasks[id].stack_base = (uint32_t)pmm_alloc_block(); 
    uint32_t* stack = (uint32_t*)(tasks[id].stack_base + STACK_SIZE);

    // Simulando o estado inicial para o 'switch_to_task'
    *(--stack) = (uint32_t)entry_point; // Onde o programa começa (EIP)

    // Espaço para os 8 registradores do 'pusha' (eax, ecx, edx, ebx, esp, ebp, esi, edi)
    for(int i=0; i<8; i++) *(--stack) = 0;

    tasks[id].esp = (uint32_t)stack;
    tasks[id].active = 1;
    return id;
}