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
bool has_thread = false;

int kthread_init(size_t max_threads) {
    lock();
    max_t = max_threads;
    ctx_buf = kmalloc(max_t * sizeof(kt_context_t), 0);
    if(!ctx_buf) return 1;
    kprintf(LOG_INFO, "kthread", "Allocated thread buffer for %u threads at %p\r\n", max_t, ctx_buf);
    memset(ctx_buf, 0, max_t * sizeof(kt_context_t));
    c_thread = 0;
    // Establish the kernel as thread 0
    kt_context_t *kctx = &ctx_buf[0];
    kctx->thread = NULL;
    kctx->state = TS_RUNNING;
    kctx->stack_base = (void*)KSTACK_BASE;
    kctx->sp = 0;
    unlock();
    return 0;
}

int kthread_create(size_t *fd, kthread_t thread, void* arg, char priv) {
    lock();
    size_t t;
    for(t = 0; t < max_t; t++) {
        if(ctx_buf[t].state == TS_UNUSED) break;
    }
    if(t >= max_t) {
        kprintf(LOG_WARN, "kthread", "Thread limit reached\r\n");
        return 1;
    }
    char flags = 0;
    if(priv != PRIV_KERNEL) flags = MEM_USER;
    void* base = kmalloc(THREAD_STACK_SIZE, flags);
    void* kbase = kmalloc(THREAD_STACK_SIZE, 0);
    if(!base || !kbase) {
        kprintf(LOG_WARN, "kthread", "kmalloc returned NULL\r\n");
        return 1;
    }
    kt_context_t *ctx = &ctx_buf[t];
    ctx->thread = thread;
    ctx->state = TS_RUNNING;
    ctx->arg = arg;
    
    ctx->stack_base = base;
    ctx->kstack_base = kbase;
    ctx->sp = (uintptr_t)base + THREAD_STACK_SIZE;
    ctx->ksp = (uintptr_t)kbase + THREAD_STACK_SIZE;
    kt_init_context(ctx, priv);
    kprintf(LOG_INFO, "kthread", "Added thread at index %u, with sp 0x%08X (ksp 0x%08X), function at %p\r\n",
        t, ctx->sp, ctx->ksp, thread
    );
    *fd = t;
    has_thread = true;
    unlock();
    return 0;
}

void kthread_schedule(uintptr_t **curr_sp, uintptr_t **next_sp) {
    *curr_sp = 0;
    *next_sp = 0;
    if(max_t == 0) return;
    // Find next active thread
    size_t tn = c_thread;
    int i = 0;
    while(ctx_buf[tn].state != TS_RUNNING || tn == c_thread) {
        tn = (tn + 1) % max_t;
        i++;
        if(i >= max_t)
            return;
        else if(ctx_buf[tn].state == TS_DONE) {
            // If a thread has just returned, free its stack and mark it as unused
            kt_context_t *ctx = &ctx_buf[tn];
            kfree(ctx->stack_base);
            kfree(ctx->kstack_base);
            ctx->state = TS_UNUSED;
        }
    }
    kstack_update(ctx_buf[tn].ksp);
    *curr_sp = &ctx_buf[c_thread].sp;
    *next_sp = &ctx_buf[tn].sp;
    c_thread = tn;
}

void* kthread_join(size_t fd) {
    while(ctx_buf[fd].state == TS_RUNNING);
    return ctx_buf[fd].res;
}

void kthread_ret(void* res) {
    kt_context_t *ctx = &ctx_buf[c_thread];
    ctx->state = TS_DONE;
    ctx->res = res;
    kprintf(LOG_INFO, "kthread", "Thread %u exited\r\n", c_thread);
}