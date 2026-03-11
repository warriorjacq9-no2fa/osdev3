#include <string.h>
#include <stdio.h>
#include <ansi.h>
#include <io.h>
#include <drivers/serial.h>

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

void serial_setcolor(uint8_t fg, uint8_t bg) {
    char ansi_buf[32];

    sprintf(ansi_buf, ANSI_SGR, fg, bg);
    serial_puts(ansi_buf);
}