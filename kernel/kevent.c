#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <stdio.h>

static kevent_consumer_t *consumers;
static uint8_t max_consumers;
static uint8_t con_idx;

static kevent_input_t *ibuf;
static ringbuffer_t kinput_rb;

void kevent_init(size_t buf_sz, size_t num_consumers) {
    ibuf = (kevent_input_t*)kmalloc(buf_sz * sizeof(kevent_input_t));
    kprintf(LOG_INFO, "kevent", "Allocated event buffer for %u events at %p\r\n", buf_sz, ibuf);

    max_consumers = num_consumers;
    consumers = (kevent_consumer_t*)kmalloc(num_consumers * sizeof(kevent_consumer_t));
    kprintf(LOG_INFO, "kevent", "Allocated consumer array of length %u at %p\r\n", num_consumers, consumers);

    rb_init(&kinput_rb, ibuf, buf_sz, sizeof(kevent_input_t));
}

int kevent_register(kevent_consumer_t consumer) {
    if(con_idx >= max_consumers) return 1;
    consumers[con_idx] = consumer;
    con_idx++;
    return 0;
}

// It is the arch-specific code's job to call kevent_proc
// On x86 it's Timer0 on the PIT
void kevent_proc() {
    kevent_input_t evt;
    while(!rb_get(&kinput_rb, &evt)) {
        for(int i = 0; i < con_idx; i++) {
            if(consumers[i].type == evt.type)
                consumers[i].callback(&evt);
        }
    }
}

int kinput(kevent_input_t *evt) {
    return rb_put(&kinput_rb, evt);
}