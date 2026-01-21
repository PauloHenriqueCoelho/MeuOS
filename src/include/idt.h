#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// CORREÇÃO: Agora recebe um PONTEIRO (*)
typedef void (*isr_t)(registers_t*);

struct idt_entry_struct {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

void init_idt();
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif