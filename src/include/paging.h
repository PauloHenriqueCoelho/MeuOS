#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "idt.h"

// Estrutura de uma entrada de Tabela de Páginas (PTE)
typedef struct {
    uint32_t present    : 1;  // Página presente na memória
    uint32_t rw         : 1;  // Read/Write (1 = Write)
    uint32_t user       : 1;  // User/Supervisor (1 = User)
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20; // Endereço físico (shiftd >> 12)
} page_t;

// Estrutura de uma Tabela de Páginas (contém 1024 páginas)
typedef struct {
    page_t pages[1024];
} page_table_t;

// Estrutura de um Diretório de Páginas (contém 1024 tabelas)
typedef struct {
    uint32_t tablesPhysical[1024]; // Endereços físicos das tabelas (para o CR3)
    page_table_t* tables[1024];    // Ponteiros virtuais (para o kernel usar)
} page_directory_t;

extern page_directory_t* kernel_page_directory;
void enablePaging();
void paging_init();
void paging_map_page(uint32_t virt, uint32_t phys, page_directory_t* dir);
void page_fault_handler(registers_t* regs);

#endif