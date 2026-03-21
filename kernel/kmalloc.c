#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_COALESCE 65536
#define MIN_SPLIT (sizeof(kheap_header_t) + 8 + sizeof(kheap_footer_t))

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

void kheap_init() {
    heap_base = (void*)(((size_t)&__kheap_start + 0xFFF) & 0xFFFFF000);
    heap_ptr = heap_base;
    kprintf(LOG_INFO, "kmalloc", "Heap at %p (%p)\r\n", heap_ptr, &__kheap_start);
}

void* kmalloc(size_t size) {
    if(size == 0) {
#ifdef KMALLOC_DEBUG
            kprintf(LOG_WARN, "kmalloc", "Size is 0\r\n");
#endif
        return NULL;
    }
    size_t a_size = (size + 0x7) & ~0x7;
    void* c = heap_base;
    while(c < heap_ptr) {
        kheap_header_t *hdr = (kheap_header_t*)c;
        void* addr = (void*)((char*)c + sizeof(kheap_header_t));
        void* next = (void*)((char*)addr + hdr->size + sizeof(kheap_footer_t));
        if(hdr->free && hdr->size >= a_size) {
            hdr->free = false;
#ifdef KMALLOC_DEBUG
            kprintf(LOG_INFO, "kmalloc", "Found free block at %p with size %u, for size %u\r\n", addr, hdr->size, size);
#endif
            if((hdr->size - a_size) >= MIN_SPLIT) {
                kheap_footer_t *new_ftr = (kheap_footer_t*)((char*)addr + a_size);
                new_ftr->header = hdr;
                kheap_header_t *new_hdr = (kheap_header_t*)((char*)new_ftr + sizeof(kheap_footer_t));
                new_hdr->size = (hdr->size - a_size) - sizeof(kheap_header_t) - sizeof(kheap_footer_t);
                new_hdr->free = true;
                kheap_footer_t *old_ftr = (kheap_footer_t*)((char*)addr + hdr->size);
                old_ftr->header = new_hdr;
                hdr->size = a_size;
#ifdef KMALLOC_DEBUG
                kprintf(LOG_INFO, "kmalloc", "Split into new block at %p of size %u\r\n",
                    (char*)new_hdr + sizeof(kheap_header_t), new_hdr->size
                );
#endif
            }
            return addr;
        }
        c = next;
    }
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

void kfree(void* addr) {
    if(addr == NULL) return;
    // HDR is the address of the start of this block
    kheap_header_t *hdr = (kheap_header_t*)((char*)addr - sizeof(kheap_header_t));
    if(hdr->free == true) return;
    kheap_footer_t *ftr = (kheap_footer_t*)((char*)addr + hdr->size);
    hdr->free = true;
#ifdef KMALLOC_DEBUG
    size_t o_size = hdr->size;
#endif

    // Coalesce neighbors
    kheap_header_t *right = (kheap_header_t*)((char*)ftr + sizeof(kheap_footer_t));

    if((void*)right < heap_ptr && right->free && right->size <= MAX_COALESCE) {
        kheap_footer_t *ftr_right = (kheap_footer_t*)(
            (char*)right + sizeof(kheap_header_t) + right->size
        );
        hdr->size += sizeof(kheap_footer_t) + sizeof(kheap_header_t) + right->size;
        ftr = ftr_right;
        ftr->header = hdr;
#ifdef KMALLOC_DEBUG
        kprintf(LOG_INFO, "kmalloc", "Coalesced block on right of size %u\r\n", right->size, addr);
#endif
    }
    
    if((void*)hdr > heap_base) {
        kheap_footer_t *ftr_left = (kheap_footer_t*)((char*)hdr - sizeof(kheap_footer_t));
        kheap_header_t *left = ftr_left->header;
        if(left->free && left->size <= MAX_COALESCE) {
    #ifdef KMALLOC_DEBUG
            kprintf(LOG_INFO, "kmalloc", "Coalesced block on left of size %u\r\n", left->size, addr);
    #endif
            left->size += sizeof(kheap_footer_t) + sizeof(kheap_header_t) + hdr->size;
            hdr = left;
            ftr->header = left;
        }
    }

#ifdef KMALLOC_DEBUG
    if(ftr->header != hdr) {
        kprintf(LOG_WARN, "kmalloc", "Footer points to %p, expected %p\r\n", ftr->header, hdr);
    }
#endif

#ifdef KMALLOC_DEBUG
    kprintf(LOG_INFO, "kmalloc", "Freed block of size %u, total size %u at %p\r\n", o_size, hdr->size, addr);
#endif
}