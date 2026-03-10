#include <string.h>
#include <drivers/serial.h>
#include <io.h>

void serial_init() {
    outb(COM1 + 1, 0x00); // Disable interrupts

    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x01); // Divisor low (115200 baud)
    outb(COM1 + 1, 0x00); // Divisor high

    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_putc(char c) {
    while(!(inb(COM1 + 5) & 0x20));
    outb(COM1, c);
}

void serial_puts(const char* s) {
    while(*s) {
        serial_putc(*s);
        s++;
    }
}

void serial_setcolor(const char* fg, const char* bg) {
    char ansi_buf[32] = 0;
    const char* ansi = ansi_buf;

    size_t len = strlen(fg);

    strcpy(ansi, fg);
    ansi += len;

    len = strlen(bg);

    strcpy(ansi, bg);
    ansi += len;

    while(*ansi) {
        outb(COM1, *ansi);
        ansi++;
    }
}