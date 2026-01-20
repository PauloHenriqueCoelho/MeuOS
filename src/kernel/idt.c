#include "../include/idt.h"
#include "../include/io.h"
#include "../include/vga.h"
#include "../include/timer.h" // Se precisar

// Estruturas
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

extern void idt_flush(uint32_t);
extern void isr0();  // Timer (IRQ 0)
extern void isr1();  // Teclado
extern void isr12(); // Mouse (NOVO)

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags; 
}

void init_pic() {
    // Inicialização (ICW1)
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    // ICW2: Remapeia IRQ 0-7 -> 0x20-0x27 e IRQ 8-15 -> 0x28-0x2F
    outb(0x21, 0x20); io_wait(); 
    outb(0xA1, 0x28); io_wait();

    // ICW3: Conexão Mestre/Escravo
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    // ICW4: Modo 8086
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // --- MÁSCARA DE INTERRUPÇÕES (OCW1) ---
    // Precisamos habilitar:
    // IRQ 1 (Teclado) -> Bit 1 do Mestre
    // IRQ 2 (Cascata) -> Bit 2 do Mestre (Necessário para ouvir o Mouse)
    // IRQ 12 (Mouse)  -> Bit 4 do Escravo
    
    // Mestre: 1111 1001 = 0xF9 (Habilita Teclado e Cascata)
    outb(0x21, 0xF8); // <--- MUDOU AQUI: 0xF8 habilita Timer, Teclado e Cascata    
    // Escravo: 1110 1111 = 0xEF (Habilita Mouse IRQ 12)
    outb(0xA1, 0xEF); 
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Limpa IDT
    for (int i=0; i<256; i++) {
        idt_set_gate(i, 0, 0, 0); 
    }

    init_pic();
    idt_set_gate(32, (uint32_t)isr0, 0x08, 0x8E);

    // Registra Teclado (Int 33 = 0x20 + 1)
    idt_set_gate(33, (uint32_t)isr1, 0x08, 0x8E);
    
    // Registra Mouse (Int 44 = 0x20 + 0x08 start do slave + 4 offset = 44)
    // IRQ 12 mapeia para interrupção 44 (0x2C)
    idt_set_gate(44, (uint32_t)isr12, 0x08, 0x8E);

    idt_flush((uint32_t)&idt_ptr);
}