#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <arch.h>
#include <stdio.h>

void kconsumer_char(kevent_input_t *evt) {
    putc(evt->ch.character);
}

void kmain() {
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
}