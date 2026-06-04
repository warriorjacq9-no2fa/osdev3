#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kshell.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>
#include <kernel/initcall.h>
#include <drivers/ata.h>
#include <drivers/pci.h>
#include <fs/ext2.h>
#include <mm.h>
#include <arch.h>
#include <fs/vfs.h>

extern uintptr_t __initcall_start;
extern uintptr_t __initcall_end;

void kmain() {
    // Run init functions
    initcall_t* call = (initcall_t*)&__initcall_start;
    initcall_t* end = (initcall_t*)&__initcall_end;
    while(call < end) {
        if(*call) (*call)();
        call++;
    }
    kprintf(LOG_INFO, "kernel", "Ran inits from %p to %p\r\n", &__initcall_start, &__initcall_end);
    
    pci_enumerate();

    size_t efd;
    kthread_create(&efd, kevent_proc, NULL, PRIV_KERNEL);
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");

    int res = ext2_init(0);

    if(res < 0) {
        kprintf(LOG_WARN, "kernel", "ext2_init returned %d\r\n", res);
    }

    kshell_init(256, 16);
    while(1) wait();
}