#include "../include/idt.h"
#include "../include/utils.h"
#include "../include/io.h"

// Importante: Garantir que o C saiba que syscall_handler usa ponteiro
extern void syscall_handler(registers_t* regs);

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;
isr_t interrupt_handlers[256]; // isr_t deve ser void (*)(registers_t*)

extern void idt_flush(uint32_t);

extern void isr0(); extern void isr1(); extern void isr14(); extern void isr128();
extern void irq0(); extern void irq1(); extern void irq8();
extern void irq12(); extern void irq14(); extern void irq15();

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// CORREÇÃO: Recebe ponteiro (*) para bater com o Assembly
void isr_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }

    if (regs->int_no >= 32) {
        if (regs->int_no >= 40) outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
    memset(&interrupt_handlers, 0, sizeof(isr_t)*256);

    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x0);  outb(0xA1, 0x0);

    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    // Syscall
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0x8E);
    register_interrupt_handler(128, syscall_handler); // <--- Vital

    idt_flush((uint32_t)&idt_ptr);
}