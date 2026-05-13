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

    int res = ext2_init(ata_read, 0);

    if(res < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_init returned error");
        goto ret;
    }

    vnode_t fd;
    if((res = ext2_open(&fd, "/arch/x86/include/ctx.h", O_RDONLY)) < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_open failed with code %d\r\n", res);
        goto ret;
    }

    stat_t stats;
    if((res = fd.ops->fstat(&fd, &stats))) {
        kprintf(LOG_WARN, "kernel", "ext2_fstat failed with code %d\r\n", res);
        goto ret;
    }

    kprintf(LOG_INFO, "kernel", "Stats: inode %u, mode %04o, links %u, uid:gid %04u:%04u\r\n",
        stats.st_ino, stats.st_mode & 0x1FF, stats.st_nlink, stats.st_uid, stats.st_gid
    );
    kprintf(LOG_INFO, "kernel", "Stats: size %u, block size %u, blocks %u\r\n",
        stats.st_size, stats.st_blksize, stats.st_blocks
    );
    kprintf(LOG_INFO, "kernel", "Stats: atime %u, mtime %u, ctime %u\r\n",
        stats.st_atime, stats.st_mtime, stats.st_ctime
    );
    
    void* buf = kmalloc(64, 0);
    if((res = fd.ops->read(&fd, buf, 0, 64)) < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_read failed with code %d\r\n", res);
        goto ret;
    }

    kprintf(LOG_INFO, "kernel", "Read %.*s\r\n", 64, (char*)buf);

ret:
    while(1) wait();
}