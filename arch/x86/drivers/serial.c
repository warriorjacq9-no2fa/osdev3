#include <drivers/serial.h>

static s_callback_t callback;

void serial_init(s_callback_t cb) {
    callback = cb;

}