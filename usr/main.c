#include <kernel/kcall.h>
#include <string.h>

void* main(void* a) {
    const char* msg = "Hello from Usermode!\r\n";
    kcall(KCALL_PRINT, (uintptr_t)msg, 23, 0, 0);
    kcall(KCALL_EXIT, 0, 0, 0, 0);
    return a;
}