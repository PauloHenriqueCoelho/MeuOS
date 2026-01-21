#include "../include/task.h"
#include "../include/memory.h"
#include "../include/utils.h"

task_t tasks[MAX_TASKS];
int current_task = 0;
int task_count = 1;

// O trampolim que criamos no Assembly
extern void task_start_stub();

void task_init() {
    for(int i=0; i<MAX_TASKS; i++) tasks[i].active = 0;
    tasks[0].active = 1; 
}

int task_create(void* entry_point) {
    int id = -1;
    for(int i = 1; i < MAX_TASKS; i++) {
        if(!tasks[i].active) { id = i; break; }
    }
    if (id == -1) return -1;

    // Aloca pilha de 4KB
    tasks[id].stack_base = (uint32_t)pmm_alloc_block(); 
    uint32_t* stack = (uint32_t*)(tasks[id].stack_base + STACK_SIZE);

    // --- A MÁGICA DO TRAMPOLIM ---
    // Quando switch_to_task fizer 'ret', ele vai pular para cá:
    *(--stack) = (uint32_t)task_start_stub; 

    // Agora simulamos o 'popa' que acontece antes do 'ret' no switch_to_task.
    // A ordem de 'popa' é: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    // O último a sair é o EAX. O stub espera que o EAX contenha o entry_point.
    
    *(--stack) = (uint32_t)entry_point; // EAX (Será usado pelo stub: call eax)
    *(--stack) = 0; // ECX
    *(--stack) = 0; // EDX
    *(--stack) = 0; // EBX
    *(--stack) = 0; // ESP (Ignorado pelo popa)
    *(--stack) = 0; // EBP
    *(--stack) = 0; // ESI
    *(--stack) = 0; // EDI

    tasks[id].esp = (uint32_t)stack;
    tasks[id].active = 1;
    return id;
}