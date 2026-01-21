#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Cada entrada na IDT tem 8 bytes
struct idt_entry_struct {
    uint16_t base_lo;             // Primeiros 16 bits do endereço da função handler
    uint16_t sel;                 // Seletor do Segmento de Código (do nosso GDT)
    uint8_t  always0;             // Sempre zero
    uint8_t  flags;               // Flags (Presente? Ring 0 ou 3?)
    uint16_t base_hi;             // Últimos 16 bits do endereço
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

// Funções para carregar a IDT
void init_idt();
#endif