#ifndef MM_H
#define MM_H

#include <stdint.h>

#define MEM_RW          0x02
#define MEM_USER        0x04
#define MEM_CACHELESS   0x10

void mm_init();
void* alloc_page();
int map_page(void* phys, void* virt, uint8_t flags);
int page_fault(uint32_t cr2, uint32_t flags);

#endif