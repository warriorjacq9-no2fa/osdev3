#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <kernel/initcall.h>
#include <stdio.h>

#define BUF_SIZE 16
#define MAX_CONSUMERS 8

void _kevent_init();
static initcall_t kevent_init __initcall_1 = _kevent_init;

static kevent_consumer_t *consumers;
static uint8_t max_consumers;
static uint8_t con_idx;

static kevent_input_t *ibuf;
static ringbuffer_t kinput_rb;

void _kevent_init() {
    ibuf = kmalloc(BUF_SIZE * sizeof(kevent_input_t), 0);
    if(!ibuf) return; // TODO: panic
    kprintf(LOG_INFO, "kevent", "Allocated event buffer for %u events at %p\r\n", BUF_SIZE, ibuf);

    max_consumers = MAX_CONSUMERS;
    consumers = kmalloc(MAX_CONSUMERS * sizeof(kevent_consumer_t), 0);
    if(!consumers) return;
    kprintf(LOG_INFO, "kevent", "Allocated consumer array of length %u at %p\r\n", MAX_CONSUMERS, consumers);

    rb_init(&kinput_rb, ibuf, BUF_SIZE, sizeof(kevent_input_t));
    return;
}

int kevent_register(kevent_consumer_t consumer) {
    if(con_idx >= max_consumers) return 1;
    consumers[con_idx] = consumer;
    con_idx++;
    return 0;
}

void* kevent_proc(void* arg) {
    while(1) {
        kevent_input_t evt;
        while(!rb_get(&kinput_rb, &evt)) {
            for(int i = 0; i < con_idx; i++) {
                if(consumers[i].type == evt.type)
                    consumers[i].callback(&evt);
            }
        }
    }
}

int kinput(kevent_input_t *evt) {
    return rb_put(&kinput_rb, evt);
}