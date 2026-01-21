#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Estrutura de uma entrada na GDT (8 bytes)
struct gdt_entry_struct {
    uint16_t limit_low;     // Os 16 bits baixos do limite
    uint16_t base_low;      // Os 16 bits baixos da base
    uint8_t  base_middle;   // Os próximos 8 bits da base
    uint8_t  access;        // Flags de acesso (quem pode usar?)
    uint8_t  granularity;   // Granularidade e bits altos do limite
    uint8_t  base_high;     // Os últimos 8 bits da base
} __attribute__((packed));  // 'packed' impede o compilador de alinhar bytes e estragar a estrutura

typedef struct gdt_entry_struct gdt_entry_t;

// Ponteiro especial para a instrução LGDT
struct gdt_ptr_struct {
    uint16_t limit;         // Tamanho da tabela - 1
    uint32_t base;          // Endereço da primeira entrada
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// Função de inicialização
void init_gdt();

#endif