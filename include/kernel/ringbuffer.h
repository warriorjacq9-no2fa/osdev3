#ifndef RINGBUFFER_H
#define RINGBUFFR_H

#include <stdint.h>
#include <stddef.h>

typedef struct ringbuffer {
    uint8_t *data;
    size_t head;
    size_t tail;
    size_t size;
} ringbuffer_t;

void rb_init(ringbuffer_t *rb, uint8_t* data, size_t size);
int rb_empty(ringbuffer_t *rb);
int rb_full(ringbuffer_t *rb);
int rb_put(ringbuffer_t *rb, uint8_t data);
int rb_get(ringbuffer_t *rb, uint8_t *data);

#endif