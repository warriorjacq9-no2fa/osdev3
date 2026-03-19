#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdbool.h>
#include <stdint.h>

#define KMALLOC_DEBUG

typedef struct kheap_header {
    size_t size;
    bool free;
} kheap_header_t;

typedef struct kheap_footer {
    kheap_header_t* header;
} kheap_footer_t;

extern size_t __kheap_start;

static void* heap_ptr;
static void* heap_base;

static size_t has_free;

void kheap_init() {
    heap_base = (void*)(((size_t)&__kheap_start + 0xFFF) & 0xFFFFF000);
    heap_ptr = heap_base;
    has_free = 0;
    kprintf(LOG_INFO, "kmalloc", "Heap at %p (%08X)\r\n", heap_ptr, &__kheap_start);
}

void* kmalloc(size_t size) {
    if(size == 0) {
#ifdef KMALLOC_DEBUG
            kprintf(LOG_WARN, "kmalloc", "Size is 0\r\n");
#endif
        return NULL;
    }
    
    if(has_free) {
        kheap_footer_t *ftr = (kheap_footer_t*)((char*)heap_ptr - sizeof(kheap_footer_t));
        kheap_header_t *hdr = (kheap_header_t*)ftr->header;
        while(hdr) {
            if(hdr->free && hdr->size >= size) break;
            ftr = (kheap_footer_t*)((char*)hdr - sizeof(kheap_footer_t));
            hdr = (kheap_header_t*)ftr->header;
        }
        if(hdr) {
            has_free--;
            hdr->free = false;
            void* addr = (void*)((char*)hdr + sizeof(kheap_header_t));
#ifdef KMALLOC_DEBUG
            kprintf(LOG_INFO, "kmalloc", "Found free block at %p with size %u, for size %u\r\n", addr, hdr->size, size);
#endif
            return addr;
        } else {
#ifdef KMALLOC_DEBUG
            kprintf(LOG_WARN, "kmalloc", "No free block found but has_free is %u\r\n", has_free);
#endif
            return NULL;
        }
    } else {
        size_t a_size = (size + 0xF) & ~0xF;
        kheap_header_t *hdr = (kheap_header_t*)heap_ptr;
        hdr->free = false;
        hdr->size = a_size;
        heap_ptr += sizeof(kheap_header_t);

        void* addr = heap_ptr;
        heap_ptr += a_size;

        kheap_footer_t *ftr = (kheap_footer_t*)heap_ptr;
        ftr->header = hdr;
        heap_ptr += sizeof(kheap_footer_t);
#ifdef KMALLOC_DEBUG
        kprintf(LOG_INFO, "kmalloc", "Created block at %p with size %u\r\n", addr, a_size);
#endif
        return addr;
    }
}

void kfree(void* addr) {
    if(addr == NULL) return;
    kheap_header_t *hdr = (kheap_header_t*)((uint32_t)addr - sizeof(kheap_header_t));
    hdr->free = true;
    has_free++;
#ifdef KMALLOC_DEBUG
    kprintf(LOG_INFO, "kmalloc", "Freed block of size %u at %p\r\n", hdr->size, addr);
#endif
}