#include <mm.h>
#include <arch.h>
#include <string.h>

#define P_PRESENT   0x01
#define P_ACCESSED  0x20
#define P_DIRTY     0x40
#define P_1MB       0x80
#define P_GLOBAL    0x100

#define PAGE_SIZE   0x1000
#define MAX_PAGES   (2^32 / PAGE_SIZE)

#define PHYS(addr) ((uint32_t)addr - 0xC0000000)

#define i_pagedir(n) ((n >> 22) & 0x3FF)
#define i_pagetab(n) ((n >> 12) & 0x3FF)

static uint32_t pagedir[1024] __attribute__((aligned(4096)));
static uint32_t pagetab_id[1024] __attribute__((aligned(4096)));

// The main bitmap for all memory
static uint32_t bitmap[MAX_PAGES / 32];

void mm_init() {
    memset(pagedir, 0, sizeof(pagedir));

    // Identity map the lower pagetable
    // and map the higher half
    for(int i = 0; i < 1024; i++) {
        pagetab_id[i] = (i * PAGE_SIZE) | P_PRESENT | MEM_RW;
    }
    pagedir[0] = ((uint32_t)pagetab_id) | P_PRESENT | MEM_RW;
    pagedir[i_pagedir(0xC0000000)] = ((uint32_t)pagetab_id) | P_PRESENT | MEM_RW;

    set_cr3((uint32_t)pagedir);
    set_cr0(get_cr0() | 0x80000000);
}

void* alloc_page() {
    for(int i = 0; i < sizeof(bitmap); i++) {
        if(~bitmap[i]) {
            int bit = __builtin_ctz(~bitmap[i]);
            bitmap[i] |= (1 << bit);
            return (void*)((i * 32 + bit) * PAGE_SIZE);
        }
    }
    return 0;
}