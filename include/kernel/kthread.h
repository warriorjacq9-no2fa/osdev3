#ifndef KTHREAD_H
#define KTHREAD_H

#include <stdint.h>
#include <stddef.h>

typedef void*(*kthread_t)(void*);

int kthread_create(kthread_t thread, void* arg);
void kthread_schedule();
int kthread_init(size_t max_threads);

#endif