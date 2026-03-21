#include <kernel/kthread.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <arch.h>
#include <ctx.h>

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
    kctx->state = TS_RUNNING;
    kctx->stack_base = (void*)KSTACK_BASE;
    kctx->sp = 0;
    return 0;
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
    ctx->state = TS_RUNNING;
    ctx->stack_base = base;
    ctx->sp = (uintptr_t)base + THREAD_STACK_SIZE;
    kt_init_context(ctx, arg);
    kprintf(LOG_INFO, "kthread", "Added thread at index %u, with stack at %p (sp %p), function at %p\r\n",
        num_t, base, ctx->sp, thread
    );
    num_t++;
    return 0;
}

void kthread_schedule() {
    size_t thread_curr = c_thread;
    size_t thread_next = (c_thread + 1) % num_t;
    c_thread = thread_next;
    if(ctx_buf[c_thread].state == TS_DONE) {
        kt_context_t *ctx = &ctx_buf[c_thread];
        kfree(ctx->stack_base);
    } else {
        kprintf(LOG_INFO, "kthread", "Thread %u scheduled\r\n",
            thread_next
        );
        ctx_switch(&ctx_buf[thread_curr].sp, ctx_buf[thread_next].sp);
    }
}

// This function is returned to by an exiting thread
void kthread_ret() {
    kt_context_t *ctx = &ctx_buf[c_thread];
    ctx->state = TS_DONE;
    kprintf(LOG_INFO, "kthread", "Thread %u returned\r\n",
        c_thread
    );
    kthread_schedule();
}