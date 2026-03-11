#include <kernel/ringbuffer.h>
#include <string.h>

void rb_init(ringbuffer_t *rb, uint8_t* data, size_t size) {
    rb->data = data;
    rb->head = 0;
    rb->tail = 0;
    rb->size = size;
}

int rb_empty(ringbuffer_t *rb) {
    return rb->head == rb->tail;
}

int rb_full(ringbuffer_t *rb) {
    return ((rb->head + 1) % rb->size) == rb->tail;
}

int rb_put(ringbuffer_t *rb, uint8_t data) {
    if(rb_full(rb)) return 1;
    rb->data[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;
    return 0;
}

int rb_get(ringbuffer_t *rb, uint8_t *data) {
    if(rb_empty(rb)) return 1;
    *data = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return 0;
}