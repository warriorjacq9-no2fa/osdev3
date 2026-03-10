#include <kernel/klog.h>

void kmain() {
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
}