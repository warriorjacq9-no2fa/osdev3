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
}