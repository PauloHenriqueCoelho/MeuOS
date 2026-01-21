#include "../include/fs.h"
#include "../include/ata.h"
#include "../include/vga.h"
#include "../include/utils.h"
#include "../include/memory.h" // Para pmm_alloc se necessário, mas aqui usaremos buffer direto

FileEntry file_table[MAX_FILES]; 

// --- Funções Privadas ---
void fs_save_table() {
    uint8_t raw_buffer[512];
    // Limpa buffer
    for(int i=0; i<512; i++) raw_buffer[i] = 0;
    
    // Copia a tabela para o buffer
    // Nota: MAX_FILES * sizeof(FileEntry) deve ser <= 512. 
    // 10 * 44 = 440 bytes. OK.
    FileEntry* entries = (FileEntry*)raw_buffer;
    for(int i=0; i<MAX_FILES; i++) entries[i] = file_table[i];
    
    ata_write_sector(TABLE_SECTOR, raw_buffer);
}

void fs_load_table() {
    uint8_t raw_buffer[512];
    ata_read_sector(TABLE_SECTOR, raw_buffer);
    FileEntry* entries = (FileEntry*)raw_buffer;
    for(int i=0; i<MAX_FILES; i++) file_table[i] = entries[i];
}

// --- Funções Públicas ---

void fs_format() {
    for(int i=0; i<MAX_FILES; i++) {
        file_table[i].used = 0;
        file_table[i].name[0] = '\0';
    }
    fs_save_table();
}

void fs_list() {
    fs_load_table();
    int found = 0;
    vga_print("Arquivos no Disco:\n");
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1) {
            vga_print("- "); 
            vga_print(file_table[i].name); 
            vga_print("\n");
            found++;
        }
    }
    if(found == 0) vga_print("(Disco Vazio)\n");
}

int fs_create(char* name, char* content) {
    fs_load_table();
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) return 0;
    }

    int free_slot = -1;
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 0) { free_slot = i; break; }
    }
    if(free_slot == -1) return 0; 

    // CÁLCULO DE SETOR CORRIGIDO: Espaçamento de 64 setores
    uint32_t sector = DATA_START_SECTOR + (free_slot * SECTORS_PER_FILE);
    
    file_table[free_slot].used = 1;
    file_table[free_slot].sector = sector;
    strcpy(file_table[free_slot].name, name);
    file_table[free_slot].size = strlen(content);

    // Grava apenas o primeiro setor (para arquivos de texto simples isso basta por enquanto)
    // Para o calc.bin usamos o PACKER, então essa função fs_create é só para pequenos textos do OS.
    uint8_t buffer[512];
    for(int k=0; k<512; k++) buffer[k] = 0;
    strcpy((char*)buffer, content);
    ata_write_sector(sector, buffer);
    
    fs_save_table();
    return 1;
}

int fs_read_to_buffer(char* name, char* buffer) {
    fs_load_table();
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) {
            
            // CÁLCULO DE QUANTOS SETORES LER
            int total_size = file_table[i].size;
            int num_sectors = (total_size + 511) / 512; // Arredonda para cima
            
            // Proteção: não ler mais do que o slot permite
            if (num_sectors > SECTORS_PER_FILE) num_sectors = SECTORS_PER_FILE;

            for (int s = 0; s < num_sectors; s++) {
                // Lê setor por setor e avança o buffer em 512 bytes
                ata_read_sector(file_table[i].sector + s, (uint8_t*)buffer + (s * 512));
            }

            return 1; 
        }
    }
    return 0;
}

int fs_delete(char* name) {
    fs_load_table();
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) {
            file_table[i].used = 0;
            fs_save_table();
            return 1;
        }
    }
    return 0;
}

int fs_exists(char* name) {
    fs_load_table(); 
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

// Adicione isso no final do src/kernel/fs.c

void fs_get_list_str(char* buffer) {
    fs_load_table();
    buffer[0] = '\0'; // Começa com string vazia

    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1) {
            // Busca o final da string atual no buffer
            int len = 0;
            while(buffer[len] != 0) len++;

            // Copia o nome do arquivo para o buffer
            char* name = file_table[i].name;
            int j = 0;
            while(name[j] != 0) {
                buffer[len++] = name[j++];
            }
            
            // Adiciona uma quebra de linha
            buffer[len++] = '\n';
            buffer[len] = '\0'; // Finaliza string
        }
    }
    
    // Se estiver vazio
    if (buffer[0] == '\0') {
        strcpy(buffer, "(Disco Vazio)\n");
    }
}