#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Permite que o loader.c acesse o diretório global
extern uint32_t kernel_page_directory[1024]; 

void paging_init();
void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t* directory);

#endif