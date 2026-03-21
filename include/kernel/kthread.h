#ifndef KTHREAD_H
#define KTHREAD_H

#include <stdint.h>
#include <stddef.h>

#define THREAD_STACK_SIZE 1024

typedef void*(*kthread_t)(void*);

enum ktask_state {
    TS_RUNNING,
    TS_DONE
};

typedef struct kt_context {
    kthread_t thread;
    unsigned char state;
    void* stack_base;
    uintptr_t sp;
} kt_context_t;

void kthread_ret();
int kthread_create(kthread_t thread, void* arg);
void kthread_schedule();
int kthread_init(size_t max_threads);

#endif