#ifndef KTHREAD_H
#define KTHREAD_H

#include <stdint.h>
#include <stddef.h>

#define THREAD_STACK_SIZE (16 * 1024) // 16 KiB stacks
#define PRIV_KERNEL 0
#define PRIV_USER   3

typedef void*(*kthread_t)(void*);

enum ktask_state {
    TS_UNUSED,
    TS_RUNNING,
    TS_DONE
};

typedef struct kt_context {
    kthread_t thread;
    unsigned char state;
    void* stack_base;
    uintptr_t sp;

    void* kstack_base;
    uintptr_t ksp;
} kt_context_t;

void kthread_ret();
int kthread_create(size_t *fd, kthread_t thread, void* arg, char priv);
void kthread_schedule(uintptr_t **curr_sp, uintptr_t **next_sp);
int kthread_init(size_t max_threads);

#endif