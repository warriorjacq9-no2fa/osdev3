#ifndef MM_H
#define MM_H

#include <stdint.h>

void mm_init();
void* alloc_page();
int map_page(uint32_t* pagedir, void* phys, void* virt, uint8_t flags);
int page_fault(uint32_t cr2, uint32_t flags);

#endif