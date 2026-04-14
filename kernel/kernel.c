#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>
#include <drivers/ata.h>
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
    kthread_init(3);
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
    size_t efd, ufd;
    kthread_create(&efd, kevent_proc, NULL, PRIV_KERNEL);
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
    void* ubuf = kmalloc(1024, MEM_USER);
    ata_read((32768 + 512) / 512, 2, (uint16_t*)ubuf);
    kprintf(LOG_INFO, "kernel", "Read user binary\r\n");

    kthread_create(&ufd, (kthread_t)ubuf, NULL, PRIV_USER);
    kprintf(LOG_INFO, "kernel", "Ran user binary\r\n");

    while(1) wait();
}