#include <kernel/kthread.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <arch.h>
#include <ctx.h>
#include <stdbool.h>
#include <string.h>

kt_context_t *ctx_buf;
size_t c_thread;
size_t max_t;
bool init = false;

int kthread_init(size_t max_threads) {
    max_t = max_threads + 1;
    ctx_buf = kmalloc((max_threads + 1) * sizeof(kt_context_t), 0);
    if(!ctx_buf) return 1;
    kprintf(LOG_INFO, "kthread", "Allocated thread buffer for %u threads at %p\r\n", max_threads, ctx_buf);
    memset(ctx_buf, 0, max_threads * sizeof(kt_context_t));
    c_thread = 0;
    // Establish the kernel as thread 0
    kt_context_t *kctx = &ctx_buf[0];
    kctx->thread = NULL;
    kctx->state = TS_RUNNING;
    kctx->stack_base = (void*)KSTACK_BASE;
    kctx->sp = 0;
    init = true;
    return 0;
}

int kthread_create(size_t *fd, kthread_t thread, void* arg, char priv) {
    size_t t;
    for(t = 0; t < max_t; t++) {
        if(ctx_buf[t].state == TS_DONE || ctx_buf[t].state == TS_UNUSED) break;
    }
    if(t >= max_t) {
        kprintf(LOG_WARN, "kthread", "Thread limit reached\r\n");
        return 1;
    }
    char flags = 0;
    if(priv != PRIV_KERNEL) flags = MEM_USER;
    void* base = kmalloc(THREAD_STACK_SIZE, flags);
    if(!base) {
        kprintf(LOG_WARN, "kthread", "kmalloc returned NULL\r\n");
        return 1;
    }
    kt_context_t *ctx = &ctx_buf[t];
    ctx->thread = thread;
    ctx->state = TS_RUNNING;
    ctx->stack_base = base;
    ctx->sp = (uintptr_t)base + THREAD_STACK_SIZE;
    kt_init_context(ctx, arg, priv);
    kprintf(LOG_INFO, "kthread", "Added thread at index %u, with stack at %p (sp %p), function at %p\r\n",
        t, base, ctx->sp, thread
    );
    *fd = t;
    return 0;
}

void kthread_schedule(uintptr_t **curr_sp, uintptr_t **next_sp) {
    if(!init) return;
    size_t thread_curr = c_thread;
    size_t thread_next = (c_thread + 1) % max_t;
    c_thread = thread_next;
    switch (ctx_buf[c_thread].state) {
        case TS_DONE:
            kt_context_t *ctx = &ctx_buf[c_thread];
            kfree(ctx->stack_base);
            break;
        
        case TS_RUNNING:
            *curr_sp = &ctx_buf[thread_curr].sp;
            *next_sp = &ctx_buf[thread_next].sp;

        default:
            break;
    }
}

// This function is returned to by an exiting thread
void kthread_ret() {
    kt_context_t *ctx = &ctx_buf[c_thread];
    ctx->state = TS_DONE;
    while(1);
}