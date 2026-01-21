#include "../include/paging.h"
#include "../include/memory.h"

// 4 tabelas de página cobrem 16MB (4MB cada)
uint32_t kernel_page_directory[1024] __attribute__((aligned(4096)));
uint32_t low_page_tables[4][1024] __attribute__((aligned(4096)));

void paging_init() {
    // 1. Inicializa diretório como "Não Presente"
    for(int i = 0; i < 1024; i++) {
        kernel_page_directory[i] = 0x00000002;
    }

    // 2. Mapeia os primeiros 16MB (Identity Mapping)
    for(int t = 0; t < 4; t++) {
        for(int i = 0; i < 1024; i++) {
            // Cada entrada aponta para o endereço físico correspondente
            low_page_tables[t][i] = ((t * 1024 + i) * 4096) | 3; // Present, R/W
        }
        // Coloca a tabela no diretório
        kernel_page_directory[t] = ((uint32_t)low_page_tables[t]) | 3;
    }

    // 3. Ativa a paginação
    __asm__ volatile("mov %0, %%cr3" :: "r"(kernel_page_directory));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

// Implementação da função de mapeamento dinâmico
void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t* directory) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    if (!(directory[pd_index] & 1)) {
        // Aloca nova tabela se necessário
        uint32_t* new_pt = (uint32_t*)pmm_alloc_block();
        for (int i = 0; i < 1024; i++) new_pt[i] = 0 | 2;
        directory[pd_index] = ((uint32_t)new_pt) | 3;
    }

    uint32_t* pt = (uint32_t*)(directory[pd_index] & ~0xFFF);
    pt[pt_index] = physical_addr | 3;
    
    __asm__ volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}