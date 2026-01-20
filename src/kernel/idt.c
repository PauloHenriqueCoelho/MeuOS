#include "../include/idt.h"
#include "../include/io.h"
#include "../include/vga.h"

// Estruturas da IDT
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

extern void isr_generic_stub();
extern void idt_flush(uint32_t);
extern void isr1(); 

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags; 
}

void init_pic() {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); 
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // Bloqueia TODAS as interrupções (0xFF) já que o main.c vai cuidar de tudo
    outb(0x21, 0xFF); 
    outb(0xA1, 0xFF); 
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_generic_stub, 0x08, 0x8E);
    }

    init_pic();
    // Não registramos isr1 aqui para evitar conflitos, pois estamos em polling.
    
    idt_flush((uint32_t)&idt_ptr);
}

// Handler vazio apenas para o Linker não reclamar que a função sumiu
void keyboard_handler_main() {
    // Apenas lê a porta para não travar, caso seja chamado acidentalmente
    inb(0x60);
    outb(0x20, 0x20);
}