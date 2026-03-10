#include <kernel/klog.h>

void kmain() {
    klog(LOG_INFO, "kernel", "Hello world!\r\n");
}