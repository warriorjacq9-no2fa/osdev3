#include <kernel/kcall.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>
#include <stdio.h>
#include <string.h>
#include <arch.h>

void kcall_proc(
    size_t id,
    uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
) {
    switch(id) {
        case KCALL_EXIT:
            kthread_ret();
            break;
        case KCALL_PRINT:
            if(!__kmem_is_user((void*)a1)) break;
            char* string = kmalloc(a2, 0);
            strncpy(string, (const char*)a1, a2);
            puts(string);
            break;
    }
}