#include <kernel/ringbuffer.h>
#include <string.h>

void rb_init(ringbuffer_t *rb, void *data, size_t size, size_t elem_size) {
    rb->data = (uint8_t*)data;
    rb->head = 0;
    rb->tail = 0;
    rb->size = size;
    rb->elem_size = elem_size;
}

int rb_empty(ringbuffer_t *rb) {
    return rb->head == rb->tail;
}

int rb_full(ringbuffer_t *rb) {
    return ((rb->head + 1) % rb->size) == rb->tail;
}

int rb_put(ringbuffer_t *rb, const void *elem) {
    if (rb_full(rb)) return 1;

    memcpy(&rb->data[rb->head * rb->elem_size], elem, rb->elem_size);
    rb->head = (rb->head + 1) % rb->size;

    return 0;
}

int rb_get(ringbuffer_t *rb, void *elem) {
    if (rb_empty(rb)) return 1;

    memcpy(elem, &rb->data[rb->tail * rb->elem_size], rb->elem_size);
    rb->tail = (rb->tail + 1) % rb->size;

    return 0;
}