#include "../include/fs.h"
#include "../include/api.h"

// Definimos um endereço de memória seguro para carregar o programa (ex: 1MB acima do Kernel)
#define USER_PROGRAM_ADDRESS 0x50000

void os_execute_bin(char* filename) {
    // 1. Carregar o binário do sistema de ficheiros para a RAM
    // Usamos a tua função fs_read_to_buffer
    if (fs_read_to_buffer(filename, (char*)USER_PROGRAM_ADDRESS)) {
        
        os_print(" -> A executar binario...\n");

        // 2. Criar um ponteiro de função para o endereço do programa
        void (*program_entry)() = (void (*)())USER_PROGRAM_ADDRESS;

        // 3. Saltar para o programa!
        program_entry();

        os_print("\n -> Programa terminado.\n");
    } else {
        os_print(" [Erro] Nao foi possivel carregar o binario.\n");
    }
}