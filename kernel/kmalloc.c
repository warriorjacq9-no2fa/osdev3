#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct kheap_block {
    size_t size;
    bool free;
    struct kheap_block* next;
} kheap_block_t;

extern uint32_t __kheap_start;

static void* heap_ptr;

void kheap_init() {
    heap_ptr = (void*)(((uint32_t)&__kheap_start + 0xFFF) & 0xFFFFF000);
    kprintf(LOG_INFO, "kmalloc", "Heap at %08X (%08X)\r\n", (uint32_t)heap_ptr, &__kheap_start);
}

void* kmalloc(size_t size) {
    void* addr = heap_ptr;
    heap_ptr += size;
    return addr;
}