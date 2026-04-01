#ifndef KCALL_H
#define KCALL_H

#include <stdint.h>
#include <stddef.h>

#define KCALL_PRINT 1

void kcall(
    size_t id,
    uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
);

#endif