#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096            // Blocos de 4KB
#define MAX_RAM   128 * 1024 * 1024 // Suportar até 128MB por enquanto
#define BITMAP_SIZE (MAX_RAM / PAGE_SIZE / 8)

void pmm_init(uint32_t mem_size);
void* pmm_alloc_block();          // Reserva 4KB e retorna o endereço
void pmm_free_block(void* ptr);   // Liberta o bloco

#endif