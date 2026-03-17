#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <mm.h>
#include <arch.h>
#include <stdio.h>

void kconsumer_char(kevent_input_t *evt) {
    putc(evt->ch.character);
}

void kmain() {
#ifndef __i386__
    mm_init();
#endif
    arch_init();
    kevent_init();
    kevent_consumer_t consumer = {
        .callback = kconsumer_char,
        .type = KEVENT_CHAR
    };
    if(kevent_register(consumer)) {
        kprintf(LOG_ERR, "kernel", "Failed to register consumer for char event\r\n");
        return;
    }
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
    uint32_t* t = (uint32_t*)0x400000;
    map_page((void*)t, (void*)t, MEM_RW);

    kprintf(LOG_INFO, "mm_test", "t: %08X at %08X\r\n", *t, t);
    *t = 0x1BADB002;
    kprintf(LOG_INFO, "mm_test", "t: %08X at %08X\r\n", *t, t);
}