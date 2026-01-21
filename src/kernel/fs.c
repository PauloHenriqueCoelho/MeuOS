#include "../include/fs.h"
#include "../include/ata.h"
#include "../include/vga.h"
#include "../include/utils.h"
#include "../include/memory.h"
#include "../include/api.h"

FileEntry file_table[MAX_FILES]; 

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
    vga_print("Arquivos:\n");
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1) {
            vga_print("- "); 
            vga_print(file_table[i].name); 
            vga_print("\n");
            found++;
        }
    }
    if(found == 0) vga_print("(Vazio)\n");
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

    uint32_t sector = DATA_START_SECTOR + (free_slot * SECTORS_PER_FILE);
    file_table[free_slot].used = 1;
    file_table[free_slot].sector = sector;
    strcpy(file_table[free_slot].name, name);
    file_table[free_slot].size = strlen(content);

    uint8_t buffer[512];
    for(int k=0; k<512; k++) buffer[k] = 0;
    strcpy((char*)buffer, content);
    ata_write_sector(sector, buffer);
    fs_save_table();
    return 1;
}

// FUNÇÃO DE LEITURA COM DEBUG DETALHADO
int fs_read_to_buffer(char* name, char* buffer) {
    os_print("[FS] Buscando arquivo: '"); os_print(name); os_print("'\n");
    
    fs_load_table();
    
    int found_any = 0;
    for(int i=0; i<MAX_FILES; i++) {
        // Debug: Mostra o que encontrou
        if(file_table[i].used == 1) {
            found_any = 1;
            os_print("[FS] Slot "); 
            char c = i+'0'; char s[2]={c,0}; os_print(s); 
            os_print(": '"); os_print(file_table[i].name); os_print("'\n");

            // Comparação
            if(strcmp(file_table[i].name, name) == 0) {
                os_print("[FS] ENCONTRADO! Carregando setores...\n");
                
                int total_size = file_table[i].size;
                int num_sectors = (total_size + 511) / 512;
                if (num_sectors > SECTORS_PER_FILE) num_sectors = SECTORS_PER_FILE;

                for (int s = 0; s < num_sectors; s++) {
                    ata_read_sector(file_table[i].sector + s, (uint8_t*)buffer + (s * 512));
                }
                return 1; 
            }
        }
    }
    
    if (!found_any) {
        os_print("[FS] Tabela de arquivos vazia (ou disco nao lido).\n");
    } else {
        os_print("[FS] Arquivo nao encontrado na lista acima.\n");
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
        if(file_table[i].used == 1 && strcmp(file_table[i].name, name) == 0) return 1;
    }
    return 0;
}

void fs_get_list_str(char* buffer) {
    fs_load_table();
    buffer[0] = '\0'; 
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used == 1) {
            int len = 0; while(buffer[len] != 0) len++;
            char* name = file_table[i].name;
            int j = 0; while(name[j] != 0) buffer[len++] = name[j++];
            buffer[len++] = '\n'; buffer[len] = '\0';
        }
    }
    if (buffer[0] == '\0') strcpy(buffer, "(Vazio)\n");
}