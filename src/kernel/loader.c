#include "../include/fs.h"
#include "../include/api.h"
#include "../include/paging.h"
#include "../include/memory.h"
#include "../include/task.h"
#include "../include/timer.h"

// Função auxiliar para ver endereços na tela
void debug_print_hex(uint32_t n) {
    char* hex = "0123456789ABCDEF";
    os_print("0x");
    for(int i=28; i>=0; i-=4) {
        char c[2] = { hex[(n>>i)&0xF], 0 };
        os_print(c);
    }
}

void os_execute_bin(char* filename) {
    os_print("\n[LOADER] Iniciando execucao de: "); os_print(filename); os_print("\n");

    // 1. Aloca memória física
    void* p1 = pmm_alloc_block();
    os_print("[LOADER] Pagina 1 Alocada em: "); debug_print_hex((uint32_t)p1); os_print("\n");
    
    // Aloca as outras 3 páginas
    void* p2 = pmm_alloc_block();
    void* p3 = pmm_alloc_block();
    void* p4 = pmm_alloc_block();
    
    // 2. Tenta ler o arquivo
    os_print("[LOADER] Lendo do disco...\n");
    
    int success = fs_read_to_buffer(filename, (char*)p1);
    
    if (success) {
        os_print("[LOADER] Leitura OK! Conteudo (Hex): ");
        unsigned char* buf = (unsigned char*)p1;
        debug_print_hex(buf[0]); os_print(" ");
        debug_print_hex(buf[1]); os_print(" ");
        debug_print_hex(buf[2]); os_print("\n");

        // Verifica se leu lixo (tudo zero)
        if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0) {
             os_print("[ERRO] O arquivo foi lido mas esta VAZIO (zeros)!\n");
             os_print("TRAVANDO SISTEMA PARA LEITURA...");
             while(1); // <--- TRAVA AQUI
        }

        os_print("[LOADER] Mapeando memoria virtual...\n");
        paging_map_page(0x400000, (uint32_t)p1, kernel_page_directory);
        paging_map_page(0x401000, (uint32_t)p2, kernel_page_directory);
        paging_map_page(0x402000, (uint32_t)p3, kernel_page_directory);
        paging_map_page(0x403000, (uint32_t)p4, kernel_page_directory);
        
        os_print("[LOADER] Criando Tarefa...\n");
        int pid = task_create((void*)0x400000);
        
        if (pid != -1) {
            os_print("[LOADER] Sucesso! PID: "); 
            char c = pid + '0'; char s[2] = {c, 0}; os_print(s); 
            os_print("\n[LOADER] Trocando contexto em 3... 2... 1...\n");
            sleep(300);
            
            // Se chegou aqui, o problema é no Task Switch ou no Código do App
            // Vamos deixar rodar. Se a tela limpar depois disso, o app rodou e saiu.
        } else {
            os_print("[ERRO] task_create retornou -1.\n");
            os_print("TRAVANDO SISTEMA PARA LEITURA...");
            while(1); // <--- TRAVA AQUI
        }
    } else {
        // Falha na leitura do arquivo
        pmm_free_block(p1); pmm_free_block(p2);
        pmm_free_block(p3); pmm_free_block(p4);
        os_print("[ERRO] fs_read_to_buffer falhou! Arquivo nao encontrado ou erro de disco.\n");
        os_print("TRAVANDO SISTEMA PARA LEITURA...");
        while(1); // <--- TRAVA AQUI
    }
}