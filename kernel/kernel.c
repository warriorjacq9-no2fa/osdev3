#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>
#include <drivers/ata.h>
#include <fs/ext2.h>
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
    kheap_init();
    usermode_init();
    kthread_init(16);
    kevent_init(16, 8);
    ata_init();
    kevent_consumer_t consumer = {
        .callback = kconsumer_char,
        .type = KEVENT_CHAR
    };
    if(kevent_register(consumer)) {
        kprintf(LOG_ERR, "kernel", "Failed to register consumer for char event\r\n");
        return;
    }
    size_t efd;
    kthread_create(&efd, kevent_proc, NULL, PRIV_KERNEL);
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");

    if(ext2_init(ata_read, 0))
        kprintf(LOG_WARN, "kernel", "ext2_init returned 1");

    while(1) wait();
}