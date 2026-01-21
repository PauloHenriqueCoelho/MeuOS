#include "../include/timer.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/api.h"
#include "../include/idt.h" // <--- OBRIGATÓRIO: Define 'registers_t'
#include "../include/task.h" // Necessário para ver a função scheduler


uint32_t tick = 0;

// Callback chamado a cada "batida" do relógio (IRQ 0)
void timer_callback(registers_t* regs) {
    (void)regs;
    tick++;
    outb(0x20, 0x20); // EOI manual as vezes é necessário no timer se não for pelo handler comum
    scheduler();
}

// Inicializa o PIT
void init_timer(uint32_t frequency) {
    // Registra a função de callback (ISR)
    // Nota: Precisaremos registrar isso na IDT (faremos no passo 2)
    
    // O valor que enviamos para o divisor do PIT é 1193180 / frequencia
    uint32_t divisor = 1193180 / frequency;

    // Manda o comando para a porta 0x43 (Mode Command Register)
    outb(0x43, 0x36);

    // Manda o divisor (byte baixo e depois byte alto)
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    outb(0x40, l);
    outb(0x40, h);
    
    // os_print("Timer Iniciado.\n");
}

// Função sleep baseada em interrupção (MUITO melhor que loop for)
// Trava o sistema por 'ticks' unidades de tempo
void sleep(uint32_t ticks) {
    uint32_t end_ticks = tick + ticks;
    while (tick < end_ticks) {
        __asm__ volatile("hlt"); // Economiza energia enquanto espera
    }
}

// No seu timer_handler ou similar
void scheduler() {
    int old = current_task;
    current_task = (current_task + 1) % MAX_TASKS;
    
    // Acha a próxima tarefa ativa
    while(!tasks[current_task].active) {
        current_task = (current_task + 1) % MAX_TASKS;
    }

    if (old != current_task) {
        // Se estiver trocando para o App (geralmente PID 1), imprima algo
        if (current_task == 1) {
            // Imprime um caractere discreto para não poluir muito (ex: '!')
            // Se aparecerem muitos '!', o scheduler está funcionando.
            // os_print("!"); 
        }
        
        extern void switch_to_task(uint32_t* old_esp, uint32_t new_esp);
        switch_to_task(&tasks[old].esp, tasks[current_task].esp);
    }
}

uint32_t get_tick() {
    return tick;
}