#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct ringbuffer {
    uint8_t *data;      // raw storage
    size_t head;
    size_t tail;
    size_t size;        // number of elements
    size_t elem_size;   // size of each element
} ringbuffer_t;

void rb_init(ringbuffer_t *rb, void *data, size_t size, size_t elem_size);
int rb_empty(ringbuffer_t *rb);
int rb_full(ringbuffer_t *rb);
int rb_put(ringbuffer_t *rb, const void *elem);
int rb_get(ringbuffer_t *rb, void *elem);

#endif