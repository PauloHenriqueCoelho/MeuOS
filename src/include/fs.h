#ifndef FS_H
#define FS_H
#include <stdint.h>

#define MAX_FILES 10
#define TABLE_SECTOR 1
#define DATA_START_SECTOR 2

typedef struct {
    char name[16];
    uint32_t sector;
    uint8_t used;
    uint32_t size;
} FileEntry;

void fs_format();
void fs_list();
int fs_create(char* name, char* content);
int fs_delete(char* name);
int fs_read_to_buffer(char* name, char* buffer); // <--- Atualizado
int fs_exists(char* name);
void fs_get_list_str(char* buffer);
#endif