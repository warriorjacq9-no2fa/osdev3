#ifndef CTX_H
#define CTX_H

#include <stdint.h>
#include <kernel/kthread.h>

void ctx_switch(uintptr_t*, uintptr_t);
void kt_init_context(kt_context_t *ctx, void *arg, char priv);

#endif