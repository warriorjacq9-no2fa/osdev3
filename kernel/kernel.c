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
#include <fs/vfs.h>

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

    vops_t* ext2 = ext2_init(ata_read, 0);

    if(ext2 == NULL) {
        kprintf(LOG_WARN, "kernel", "ext2_init returned null");
        goto ret;
    }

    vnode_t fd;
    int res;
    if((res = ext2->open(&fd, "/README.md", O_RDONLY)) < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_open failed with code %d\r\n", res);
        goto ret;
    }
    
    void* buf = kmalloc(64, 0);
    if((res = ext2->read(&fd, buf, 0, 64)) < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_read failed with code %d\r\n", res);
        goto ret;
    }

    kprintf(LOG_INFO, "kernel", "Read %.*s\r\n", 64, (char*)buf);

ret:
    while(1) wait();
}