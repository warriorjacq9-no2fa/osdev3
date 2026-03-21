#include <ctx.h>

void kt_init_context(kt_context_t *ctx, void *arg) {
    // Start at the top of the stack (x86 stack grows downward)
    uint32_t *sp = (uint32_t *)((uint8_t *)ctx->stack_base + THREAD_STACK_SIZE);

    // Push argument and a NULL return address, as if func() was called normally
    *--sp = (uint32_t)arg;   // argument to func
    *--sp = 0;               // fake return address (undefined behaviour on return)

    // Push func as the return address so ret in ctx_switch jumps into it
    *--sp = (uint32_t)ctx->thread;

    // Push a fake pusha frame so ctx_switch can popa into a clean register state
    *--sp = 0;  // eax
    *--sp = 0;  // ecx
    *--sp = 0;  // edx
    *--sp = 0;  // ebx
    *--sp = 0;  // esp (ignored by popa)
    *--sp = 0;  // ebp
    *--sp = 0;  // esi
    *--sp = 0;  // edi

    ctx->sp = (uint32_t)sp;
}