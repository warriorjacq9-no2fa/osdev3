#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kshell.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>
#include <drivers/ata.h>
#include <drivers/pci.h>
#include <fs/ext2.h>
#include <mm.h>
#include <arch.h>
#include <fs/vfs.h>

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

    size_t efd;
    kthread_create(&efd, kevent_proc, NULL, PRIV_KERNEL);
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");

    int res = ext2_init(ata_read, 0);

    if(res < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_init returned error");
    }
    pci_enumerate();

    kshell_init(256, 16);
    while(1) wait();
}