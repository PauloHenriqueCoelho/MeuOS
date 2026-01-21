#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TABLE_SECTOR 1
#define DATA_START_SECTOR 2
#define MAX_FILES 10 
#define SECTORS_PER_FILE 64 

// ESSENCIAL: __attribute__((packed)) para garantir compatibilidade com o Kernel
typedef struct __attribute__((packed)) {
    char name[32];
    uint32_t size;
    uint32_t sector;
    uint32_t used;
} FileEntry;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Uso: %s <disk.img> <arquivo_host> <nome_no_os>\n", argv[0]);
        return 1;
    }

    // Validação de Tamanho da Struct (Debug)
    if (sizeof(FileEntry) != 44) {
        printf("[ERRO FATAL] FileEntry tem %lu bytes no Mac. Deveria ter 44!\n", sizeof(FileEntry));
        return 1;
    }

    char* disk_name = argv[1];
    char* host_file = argv[2];
    char* os_name = argv[3];

    FILE* disk = fopen(disk_name, "rb+");
    if (!disk) { 
        // Se não existir, cria um novo zerado
        disk = fopen(disk_name, "wb+");
        if(!disk) { perror("Erro ao criar disco"); return 1; }
        // Expande para 10MB (exemplo) para evitar erros de seek
        fseek(disk, 10*1024*1024, SEEK_SET);
        fputc(0, disk);
        rewind(disk);
    }

    FileEntry table[MAX_FILES];
    memset(table, 0, sizeof(table));
    
    // Lê tabela atual
    fseek(disk, TABLE_SECTOR * 512, SEEK_SET);
    fread(table, sizeof(FileEntry), MAX_FILES, disk);

    // Procura slot
    int slot = -1;
    for(int i=0; i<MAX_FILES; i++) {
        // Usa uint32_t para comparação segura
        if (table[i].used == 1 && strncmp(table[i].name, os_name, 32) == 0) {
            slot = i;
            printf("Atualizando '%s' (Slot %d)\n", os_name, slot);
            break;
        }
        if (table[i].used == 0 && slot == -1) {
            slot = i;
        }
    }

    if (slot == -1) {
        printf("Erro: Tabela cheia!\n");
        fclose(disk);
        return 1;
    }

    // Lê binário
    FILE* f = fopen(host_file, "rb");
    if (!f) { perror("Erro ao abrir binario"); fclose(disk); return 1; }

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*)calloc(1, size + 512);
    fread(buffer, 1, size, f);
    fclose(f);

    // Atualiza Struct
    memset(table[slot].name, 0, 32);
    strncpy(table[slot].name, os_name, 31);
    table[slot].size = (uint32_t)size;
    table[slot].used = 1;
    
    // Setor fixo baseado no slot (para simplificar o FS do Kernel)
    table[slot].sector = DATA_START_SECTOR + (slot * SECTORS_PER_FILE);

    // Escreve Dados
    fseek(disk, table[slot].sector * 512, SEEK_SET);
    fwrite(buffer, 1, size, disk);

    // Escreve Tabela
    fseek(disk, TABLE_SECTOR * 512, SEEK_SET);
    fwrite(table, sizeof(FileEntry), MAX_FILES, disk);

    fclose(disk);
    free(buffer);
    printf("Sucesso! '%s' injetado (Slot %d, Setor %d, %d bytes)\n", os_name, slot, table[slot].sector, size);
    return 0;
}