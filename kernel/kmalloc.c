#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct kheap_header {
    size_t size;
    bool free;
} kheap_header_t;

typedef struct kheap_footer {
    kheap_header_t* header;
} kheap_footer_t;

extern size_t __kheap_start;

static void* heap_ptr;

static size_t has_free;

void kheap_init() {
    heap_ptr = (void*)(((size_t)&__kheap_start + 0xFFF) & 0xFFFFF000);
    has_free = 0;
    kprintf(LOG_INFO, "kmalloc", "Heap at %p (%08X)\r\n", heap_ptr, &__kheap_start);
}

void* kmalloc(size_t size) {
    size_t a_size = (size + 0xF) & ~0xF;
    if(has_free) {
        kheap_footer_t *ftr = (kheap_footer_t*)((char*)heap_ptr - sizeof(kheap_footer_t));
        kheap_header_t *hdr = (kheap_header_t*)ftr->header;
        while(hdr && !hdr->free) {
            ftr = (kheap_footer_t*)((char*)hdr - sizeof(kheap_footer_t));
            hdr = (kheap_header_t*)ftr->header;
        }
        has_free--;
        return (void*)((char*)hdr + sizeof(kheap_header_t));
    } else {
        kheap_header_t *hdr = (kheap_header_t*)heap_ptr;
        hdr->free = false;
        hdr->size = a_size;
        heap_ptr += sizeof(kheap_header_t);

        void* addr = heap_ptr;
        heap_ptr += a_size;

        kheap_footer_t *ftr = (kheap_footer_t*)heap_ptr;
        ftr->header = hdr;
        heap_ptr += sizeof(kheap_footer_t);
        return addr;
    }
}

void kfree(void* addr) {
    kheap_header_t *hdr = (kheap_header_t*)((uint32_t)addr - sizeof(kheap_header_t));
    hdr->free = true;
    has_free++;
}