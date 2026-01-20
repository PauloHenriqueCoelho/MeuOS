#include "../include/timer.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/api.h"

uint32_t tick = 0;

// Callback chamado a cada "batida" do relógio (IRQ 0)
void timer_callback() {
    tick++;
    
    // Opcional: Mostra algo na tela para provar que está vivo (ex: a cada 100 ticks)
    // if (tick % 100 == 0) {
    //    os_print("Tick! ");
    // }
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

uint32_t get_tick() {
    return tick;
}