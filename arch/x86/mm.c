#include <mm.h>
#include <arch.h>

#define P_PRESENT   0x01
#define P_RW        0x02
#define P_USER      0x04
#define P_WTHROUGH  0x08
#define P_NOCACHE   0x10
#define P_ACCESSED  0x20
#define P_DIRTY     0x40
#define P_1MB       0x80
#define P_GLOBAL    0x01

#define i_pagedir(n) ((n >> 22) & 0x3FF)
#define i_pagetab(n) ((n >> 12) & 0x3FF)

static uint32_t pagedir[1024];
static uint32_t pagetab_id[1024];

void mm_init() {

    // Identity map the lower pagetable
    for(int i = 0; i < 1024; i++) {
        pagetab_id[i] = (i * 0x1000) | P_PRESENT | P_RW;
    }
    pagedir[i_pagedir(0)] = ((uint32_t)pagetab_id) | P_PRESENT | P_RW;

    set_cr3((uint32_t)pagedir);
    set_cr0(get_cr0() | 0x80000000);
}