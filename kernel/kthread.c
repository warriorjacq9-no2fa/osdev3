#include <kernel/kthread.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <arch.h>

#define THREAD_STACK_SIZE 1024

typedef struct kt_context {
    kthread_t thread;
    void* stack_base;
    uint32_t sp;
} kt_context_t;

kt_context_t *ctx_buf;
size_t c_thread;
size_t num_t;
size_t max_t;

int kthread_init(size_t max_threads) {
    max_t = max_threads + 1;
    ctx_buf = kmalloc((max_threads + 1) * sizeof(kt_context_t));
    if(!ctx_buf) return 1;
    kprintf(LOG_INFO, "kthread", "Allocated thread buffer for %u threads at %p\r\n", max_threads, ctx_buf);
    c_thread = 0;
    // Establish the kernel as thread 0
    num_t = 1;
    kt_context_t *kctx = &ctx_buf[0];
    kctx->thread = NULL;
    kctx->stack_base = (void*)0xC0010000; // This is defined in boot.S
    kctx->sp = 0;
    return 0;
}

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

int kthread_create(kthread_t thread, void* arg) {
    if(num_t >= max_t) {
        kprintf(LOG_WARN, "kthread", "Thread limit reached\r\n");
        return 1;
    }
    kt_context_t *ctx = &ctx_buf[num_t];
    ctx->thread = thread;
    void* base = kmalloc(THREAD_STACK_SIZE);
    if(!base) {
        kprintf(LOG_WARN, "kthread", "kmalloc returned NULL\r\n");
        return 1;
    }
    ctx->stack_base = base;
    ctx->sp = (uint32_t)base + THREAD_STACK_SIZE;
    kt_init_context(ctx, arg);
    kprintf(LOG_INFO, "kthread", "Added thread at index %u, with stack at %p (sp %p), function at %p\r\n",
        num_t, base, ctx->sp, thread
    );
    num_t++;
    return 0;
}

extern void ctx_switch(uint32_t*, uint32_t);

void kthread_schedule() {
    size_t thread_curr = c_thread;
    size_t thread_next = (c_thread + 1) % num_t;
    c_thread = thread_next;
    kprintf(LOG_INFO, "kthread", "Thread %u scheduled\r\n",
        thread_next
    );
    ctx_switch(&ctx_buf[thread_curr].sp, ctx_buf[thread_next].sp);
}