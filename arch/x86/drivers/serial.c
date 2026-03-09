#include <drivers/serial.h>
#include <io.h>

static s_callback_t callback;

void serial_init(s_callback_t cb) {
    callback = cb;
    outb(COM1+3, 0b10000111); // 8N1, DLAB
    outb(COM1+1, 0); // Divisor MSB: 0
    outb(COM1, 1); // Divisor LSB: 1
    outb(COM1+3, 0b00000111); // 8N1, no DLAB
}

void serial_putc(char c) {
    outb(COM1, c);
}

void serial_puts(const char* s) {
    while(*s) {
        serial_putc(*s);
        s++;
    }
}

void serial_setcolor(const char* ansi) {
    while(*ansi) {
        outb(COM1, *ansi);
        ansi++;
    }
}