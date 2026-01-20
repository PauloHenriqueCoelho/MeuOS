#include "../include/fs.h"
#include "../include/ata.h"
#include "../include/vga.h"
#include "../include/utils.h"

FileEntry file_table[MAX_FILES]; 

// Funções Internas (Privadas)
void fs_save_table() {
    uint8_t raw_buffer[512];
    for(int i=0; i<512; i++) raw_buffer[i] = 0;
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

// --- Implementação Pública ---

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
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1) {
            vga_print("- "); vga_print(file_table[i].name); vga_print("\n");
            found++;
        }
    }
    if(found == 0) vga_print("(Disco Vazio)\n");
}

int fs_create(char* name, char* content) {
    fs_load_table();
    
    // Verifica duplicatas
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) return 0; // Erro: Já existe
    }

    int free_slot = -1;
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 0) { free_slot = i; break; }
    }
    if(free_slot == -1) return 0; // Erro: Disco cheio

    uint32_t sector = DATA_START_SECTOR + free_slot;
    file_table[free_slot].used = 1;
    file_table[free_slot].sector = sector;
    strcpy(file_table[free_slot].name, name);
    file_table[free_slot].size = strlen(content);

    uint8_t buffer[512];
    for(int k=0; k<512; k++) buffer[k] = 0;
    strcpy((char*)buffer, content);
    ata_write_sector(sector, buffer);
    fs_save_table();
    return 1; // Sucesso
}

// NOVA FUNÇÃO: Lê para um buffer (API Friendly)
int fs_read_to_buffer(char* name, char* buffer) {
    fs_load_table();
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) {
            ata_read_sector(file_table[i].sector, (uint8_t*)buffer);
            return 1; // Encontrou
        }
    }
    return 0; // Não encontrou
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