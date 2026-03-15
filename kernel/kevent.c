#include <kernel/kevent.h>
#include <kernel/klog.h>
#include <kernel/ringbuffer.h>
#include <stdio.h>

static kevent_consumer_t consumers[8];
static uint8_t con_idx;

static kevent_input_t ibuf[32];
static ringbuffer_t kinput_rb;

void kevent_init() {
    rb_init(&kinput_rb, ibuf, sizeof(ibuf) / sizeof(kevent_input_t), sizeof(kevent_input_t));
}

int kevent_register(kevent_consumer_t consumer) {
    if(con_idx >= 8) return 1;
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