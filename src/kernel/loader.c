#include "../include/fs.h"
#include "../include/api.h"
#include "../include/paging.h" // Adicione este include
#include "../include/memory.h" // Adicione este include

// Definimos um endereço de memória seguro para carregar o programa (ex: 1MB acima do Kernel)
void os_execute_bin(char* filename) {
    // Aloca endereço FÍSICO
    void* phys_address = pmm_alloc_block();
    
    if (fs_read_to_buffer(filename, (char*)phys_address)) {
        // Mapeia Virtual 0x400000 para o Físico obtido
        paging_map_page(0x400000, (uint32_t)phys_address, kernel_page_directory);
        
        // Executa no endereço VIRTUAL
        void (*program_entry)() = (void (*)())0x400000;
        program_entry();
        
        os_print("\n -> Programa concluido.\n");
    } else {
        pmm_free_block(phys_address);
        os_print(" [Erro] Arquivo nao encontrado.\n");
    }
}