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

typedef struct pd_entry {
    uint8_t flags;
    unsigned int ext_flags : 4;
    unsigned int addr : 20;
} __attribute__((packed)) page_entry_t;

static page_entry_t pagedir[1024];
static page_entry_t pagetab[1024];



void mm_init() {
    page_entry_t entry = {
        .flags = P_RW | P_PRESENT,
        .ext_flags = 0,
        .addr = ((uint32_t)&pagetab[0]) >> 12
    };
    pagedir[i_pagedir(0x7E00)] = entry;

    entry.addr = (0x7E00) >> 12;

    pagetab[i_pagetab(0x7E00)] = entry;

    set_cr3(pagedir);
    set_cr0(get_cr0() | 0x80000001);
}