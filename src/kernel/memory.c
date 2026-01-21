#include "../include/memory.h"
#include "../include/utils.h"

uint8_t bitmap[BITMAP_SIZE];
uint32_t total_blocks;

void pmm_init(uint32_t mem_size) {
    total_blocks = mem_size / PAGE_SIZE;
    for (int i = 0; i < BITMAP_SIZE; i++) bitmap[i] = 0;
    
    // Bloquear os primeiros 8MB (2048 blocos de 4KB)
    // Isso protege o Kernel (em 1MB) e as tabelas de página
    for (int i = 0; i < 2048; i++) {
        uint32_t byte = i / 8;
        uint32_t bit = i % 8;
        bitmap[byte] |= (1 << bit);
    }
}

void* pmm_alloc_block() {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            bitmap[i / 8] |= (1 << (i % 8));
            return (void*)(i * PAGE_SIZE);
        }
    }
    return 0;
}

void pmm_free_block(void* ptr) {
    uint32_t addr = (uint32_t)ptr;
    uint32_t block = addr / PAGE_SIZE;
    bitmap[block / 8] &= ~(1 << (block % 8));
}