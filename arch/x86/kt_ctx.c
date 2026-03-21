#include <ctx.h>

void kt_init_context(kt_context_t *ctx, void *arg) {
    uint32_t *sp = (uint32_t *)((uint8_t *)ctx->stack_base + THREAD_STACK_SIZE);

    *--sp = (uint32_t)arg;
    *--sp = (uint32_t)kthread_ret;

    // Push thread as the return address so ret in ctx_switch jumps into it
    *--sp = (uint32_t)ctx->thread;

    *--sp = 0;  // eax
    *--sp = 0;  // ecx
    *--sp = 0;  // edx
    *--sp = 0;  // ebx
    *--sp = 0;  // esp (ignored)
    *--sp = 0;  // ebp
    *--sp = 0;  // esi
    *--sp = 0;  // edi

    ctx->sp = (uint32_t)sp;
}