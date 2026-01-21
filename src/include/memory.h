#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096
// 128MB RAM / 4KB por página / 8 bits por byte = 4096 bytes de bitmap
#define BITMAP_SIZE 4096 

void pmm_init(uint32_t mem_size);
void* pmm_alloc_block();
void pmm_free_block(void* ptr);

#endif