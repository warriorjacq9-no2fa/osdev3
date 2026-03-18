#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <mm.h>
#include <arch.h>

void kconsumer_char(kevent_input_t *evt) {
    putc(evt->ch.character);
}

void kmain() {
#ifndef __i386__
    mm_init();
#endif
    arch_init();
    kevent_init();
    kheap_init();
    kevent_consumer_t consumer = {
        .callback = kconsumer_char,
        .type = KEVENT_CHAR
    };
    if(kevent_register(consumer)) {
        kprintf(LOG_ERR, "kernel", "Failed to register consumer for char event\r\n");
        return;
    }
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
    
    kprintf(LOG_INFO, "kernel", "kmalloc test start\r\n");

    size_t sizes[] = {
        1, 2, 3, 4,
        8, 16, 24, 32,
        64, 128, 256,
        512, 1024,
        2048, 4096,
        8192
    };

    int count = sizeof(sizes) / sizeof(size_t);

    for (int i = 0; i < count; i++) {
        size_t size = sizes[i];

        void* ptr = kmalloc(size);

        if (!ptr) {
            kprintf(LOG_ERR, "kernel", "kmalloc FAILED for size %u\r\n", size);
            continue;
        }

        kprintf(LOG_INFO, "kernel", "kmalloc(%u) = %p\r\n", size, ptr);

        // Write pattern to memory (important!)
        uint8_t* p = (uint8_t*)ptr;
        for (size_t j = 0; j < size; j++) {
            p[j] = (uint8_t)(j & 0xFF);
        }

        // Verify memory
        for (size_t j = 0; j < size; j++) {
            if (p[j] != (uint8_t)(j & 0xFF)) {
                kprintf(LOG_ERR, "kernel", "Memory corruption at size %u offset %u\r\n", size, j);
                return;
            }
        }
    }

    kprintf(LOG_INFO, "kernel", "kmalloc test finished\r\n");
}