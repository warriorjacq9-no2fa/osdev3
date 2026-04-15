#include <kernel/kcall.h>
#include <string.h>

void main(void* a) {
    const char* msg = "Hello from Usermode!\r\n";
    kcall(KCALL_PRINT, (uintptr_t)msg, 23, 0, 0);
    kcall(KCALL_EXIT, (uintptr_t)a, 0, 0, 0);
    while(1);
}