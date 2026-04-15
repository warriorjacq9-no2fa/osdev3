#ifndef KCALL_H
#define KCALL_H

#include <stdint.h>
#include <stddef.h>

#define KCALL_EXIT  1
#define KCALL_PRINT 2

void kcall(
    size_t id,
    uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
);

void kcall_proc(
    size_t id,
    uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
);

#endif