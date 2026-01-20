#include "../include/gdt.h"

// Vamos ter 5 entradas (Null, Kernel Code, Kernel Data, User Code, User Data)
// User Code/Data usaremos bem mais pra frente, mas já deixamos pronto.
gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;

// Função externa em Assembly para carregar a GDT (implementaremos a seguir)
extern void gdt_flush(uint32_t);

// Função auxiliar para preencher uma entrada
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // 0: Null Descriptor (Obrigatório)
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1: Kernel Code Segment
    // Base: 0, Limit: 4GB, Access: 0x9A (Exec/Read, Ring 0), Gran: 0xCF (4KB blocks)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: Kernel Data Segment
    // Base: 0, Limit: 4GB, Access: 0x92 (Read/Write, Ring 0), Gran: 0xCF
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 3: User Code Segment (Futuro)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 4: User Data Segment (Futuro)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Carrega a nova GDT
    gdt_flush((uint32_t)&gdt_ptr);
}