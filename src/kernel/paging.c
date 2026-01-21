#include "../include/paging.h"
#include "../include/memory.h"
#include "../include/utils.h"
#include "../include/api.h"
#include "../include/idt.h"

page_directory_t* kernel_page_directory = 0;

extern void loadPageDirectory(uint32_t*);
extern void enablePaging();

void* kmalloc_a(uint32_t size) {
    // Simples wrapper para alinhamento de página
    return pmm_alloc_block();
}

void paging_map_page(uint32_t virt, uint32_t phys, page_directory_t* dir) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (!dir->tables[pd_index]) {
        // Aloca nova tabela
        uint32_t* new_table_phys = (uint32_t*)pmm_alloc_block();
        memset(new_table_phys, 0, 4096); 

        // Registra
        dir->tables[pd_index] = (page_table_t*)new_table_phys;
        dir->tablesPhysical[pd_index] = (uint32_t)new_table_phys | 0x7; // Present, RW, User
    }

    page_table_t* table = dir->tables[pd_index];
    page_t* page = &table->pages[pt_index];
    
    page->frame = phys >> 12;
    page->present = 1;
    page->rw = 1;
    page->user = 1;
}

void paging_init() {
    // --- CORREÇÃO DO CRASH ---
    // A struct page_directory_t tem 8192 bytes (2 blocos de 4KB).
    // Precisamos alocar 2 blocos contíguos para não invadir memória alheia.
    void* block1 = pmm_alloc_block(); // Endereço Físico (Ex: 0x800000)
    void* block2 = pmm_alloc_block(); // Endereço Físico (Ex: 0x801000)
    
    // Usa o primeiro bloco como base, mas sabemos que o segundo também é nosso
    kernel_page_directory = (page_directory_t*)block1;
    
    // Agora podemos zerar 8KB sem medo de corromper o próximo bloco livre
    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Identity Map (0-16MB)
    // Isso garante que o Kernel e as próprias tabelas de página sejam acessíveis
    for (uint32_t i = 0; i < 0x1000000; i += 0x1000) {
        paging_map_page(i, i, kernel_page_directory);
    }

    register_interrupt_handler(14, page_fault_handler);

    loadPageDirectory(kernel_page_directory->tablesPhysical);
    
    // Se chegou aqui sem reiniciar, o mapeamento básico funcionou!
    // Nota: O print pode falhar se o vídeo não estiver mapeado ainda, 
    // mas o kernel_main vai cuidar disso logo em seguida.
}

void page_fault_handler(registers_t* regs) {
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // Tenta imprimir (pode falhar se for culpa do vídeo, mas ajuda no debug)
    // Usamos vga_print direto para evitar dependências de cor/janela aqui
    // Converta o endereço para Hexa mentalmente ou implemente print_hex depois
    os_print("\n[CRITICO] Page Fault detectado!\n");
    
    while(1);
}