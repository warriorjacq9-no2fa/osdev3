#include <kernel/klog.h>
#include <arch.h>

void kmain() {
    arch_init();
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
}