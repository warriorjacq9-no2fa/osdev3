#include <kernel/kcall.h>
#include <kernel/kmalloc.h>
#include <stdio.h>

void kcall(
    size_t id,
    uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
) {
    switch(id) {
        case KCALL_PRINT:
            if(!__kmem_is_user((void*)a1)) break;
            puts((char*)a1);
            break;
    }
}