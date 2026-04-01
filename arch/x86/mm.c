#include <mm.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <arch.h>
#include <string.h>
#include <stdbool.h>

#define P_PRESENT   0x01
#define P_RW        0x02
#define P_USER      0x04
#define P_ACCESSED  0x20
#define P_DIRTY     0x40
#define P_1MB       0x80
#define P_GLOBAL    0x100

#define PAGE_SIZE   0x1000
#define MAX_PAGES   ((1ULL << 32) / PAGE_SIZE)

#define PHYS(addr) ((uint32_t)addr - 0xC0000000)

#define PTAB(index) ((uint32_t*)(0xFFC00000 + index * 0x1000))

#define i_pdir(n) ((n >> 22) & 0x3FF)
#define i_ptab(n) ((n >> 12) & 0x3FF)

static uint32_t kpagedir[1024] __attribute__((aligned(4096)));
static uint32_t pagetab_id[1024] __attribute__((aligned(4096)));

// The main bitmap for all memory
static uint32_t bitmap[MAX_PAGES / 32];

void mm_init() {
    memset(kpagedir, 0, sizeof(kpagedir));

    // Identity map the lower pagetable
    // and map the higher half (1K pages)
    // Don't map the first page to catch
    // null pointer errors
    pagetab_id[0] = 0;
    for(int i = 1; i < 1024; i++) {
        pagetab_id[i] = (i * PAGE_SIZE) | P_PRESENT | P_RW;
    }
    // Identity-mapped code is RO
    kpagedir[0] = ((uint32_t)pagetab_id) | P_PRESENT;
    kpagedir[i_pdir(0xC0000000)] = ((uint32_t)pagetab_id) | P_PRESENT | P_RW;

    // Recursively map the page directory
    // so we can access already assigned page tables much easier
    kpagedir[1023] = ((uint32_t)kpagedir) | P_PRESENT | P_RW;

    // Mark first 1K pages as used in the bitmap
    for(int i = 0; i < 1024 / 32; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    set_cr3((uint32_t)kpagedir);
    set_cr0(get_cr0() | 0x80000000);
}

void* alloc_page() {
    for(int i = 0; i < MAX_PAGES / 32; i++) {
        if(~bitmap[i]) {
            int bit = __builtin_ctz(~bitmap[i]);
            bitmap[i] |= (1 << bit);
            return (void*)((i * 32 + bit) * PAGE_SIZE);
        }
    }
    return 0;
}

int map_page(uint32_t* pagedir, void* phys, void* virt, uint8_t flags) {
    if(phys == NULL || virt == NULL)
        return 1;
    uint32_t vaddr = (uint32_t)virt;
    if(!(pagedir[i_pdir(vaddr)] & P_PRESENT)) {
        // Allocate a page for the page table
        uint32_t page = (uint32_t)alloc_page();
        if(!page) return 1;
        pagedir[i_pdir(vaddr)] = page | P_PRESENT | P_RW;
        if(flags & MEM_USER) pagedir[i_pdir(vaddr)] |= P_USER;
        memset(PTAB(i_pdir(vaddr)), 0, PAGE_SIZE);
    } else if((flags & MEM_USER) && !(pagedir[i_pdir(vaddr)] & P_USER))
        return 1;
    
    uint32_t* ptab = PTAB(i_pdir(vaddr));
    ptab[i_ptab(vaddr)] = ((uint32_t)phys & 0xFFFFF000) | P_PRESENT;
    if(flags & MEM_RW) ptab[i_ptab(vaddr)] |= P_RW;
    if(flags & MEM_USER) ptab[i_ptab(vaddr)] |= P_USER;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
    return 0;
}

int page_fault(uint32_t cr2, uint32_t flags) {
    if(!(flags & P_PRESENT)) {
        if(flags & P_USER) {
            if(cr2 >= 0xC0000000) return 1;
            return map_page(kpagedir, alloc_page(), (void*)cr2, MEM_RW | MEM_USER);
        }

        return map_page(kpagedir, alloc_page(), (void*)cr2, MEM_RW);
    } else return 1;
}