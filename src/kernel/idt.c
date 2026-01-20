#include "../include/idt.h"
#include "../include/io.h"
#include "../include/vga.h"

// Estruturas
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

extern void idt_flush(uint32_t);
extern void isr1(); // Handler do Assembly

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags; 
}

void init_pic() {
    // Sequência mágica de inicialização do PIC (ICW)
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    // Remapeia IRQ 0-7 para 0x20-0x27 (Para não conflitar com Exceptions 0-31)
    outb(0x21, 0x20); io_wait(); 
    outb(0xA1, 0x28); io_wait();

    // Conexão Mestre/Escravo
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    // Modo 8086
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // --- MÁSCARA DE INTERRUPÇÕES ---
    // 0 = Ligado, 1 = Desligado.
    // 0xFD = 1111 1101 (Só o bit 1 é ZERO).
    // Isso significa: Bloqueia tudo, MENOS o Teclado (IRQ 1).
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF); 
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Zera tudo primeiro
    for (int i=0; i<256; i++) {
        idt_set_gate(i, 0, 0, 0); 
    }

    // Configura o PIC
    init_pic();

    // Registra o teclado na posição 33 (0x20 + IRQ1 = 0x21 -> 33)
    idt_set_gate(33, (uint32_t)isr1, 0x08, 0x8E);

    // Carrega a tabela
    idt_flush((uint32_t)&idt_ptr);
}