#include "../include/fs.h"
#include "../include/api.h"
#include "../include/paging.h" // Adicione este include
#include "../include/memory.h" // Adicione este include
#include "../include/task.h" // Garanta que este include existe

// Definimos um endereço de memória seguro para carregar o programa (ex: 1MB acima do Kernel)
void os_execute_bin(char* filename) {
    void* phys_address = pmm_alloc_block();
    
    if (fs_read_to_buffer(filename, (char*)phys_address)) {
        paging_map_page(0x400000, (uint32_t)phys_address, kernel_page_directory);
        
        // --- A MUDANÇA É AQUI ---
        // Em vez de chamar program_entry(); 
        // Você cria uma tarefa que começa no endereço 0x400000
        task_create((void*)0x400000);
        
        // E volta imediatamente para o Shell!
        os_print(" -> Tarefa iniciada em background.\n");
    } else {
        pmm_free_block(phys_address);
        os_print(" [Erro] Arquivo nao encontrado.\n");
    }
}