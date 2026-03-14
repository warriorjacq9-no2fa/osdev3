#include <string.h>
#include <stdio.h>
#include <ansi.h>
#include <io.h>
#include <drivers/serial.h>
#include <kernel/ringbuffer.h>
#include <kernel/kevent.h>

static uint8_t buf[128];
static ringbuffer_t rb_tx;
void serial_init() {
    memset(buf, 0, sizeof(buf));
    rb_init(&rb_tx, buf, sizeof(buf), sizeof(uint8_t));
    outb(COM1 + 1, 0x00); // Disable interrupts

    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x01); // Divisor low (115200 baud)
    outb(COM1 + 1, 0x00); // Divisor high

    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(COM1 + 1, 0x03); // Enable RX IRQ, THR empty IRQ
}

void serial_putc(char c) {
    // Putc will block until the ringbuffer is not full
    // since rb_put returns 1 on failure
    while(rb_put(&rb_tx, &c));

    if (inb(COM1 + 5) & 0x20) { // THR empty
        uint8_t data;
        if (!rb_get(&rb_tx, &data))
            outb(COM1, data);
    }
}

void serial_puts(const char* s) {
    while(*s) {
        serial_putc(*s);
        s++;
    }
}

void serial_setcolor(uint8_t fg, uint8_t bg) {
    char ansi_buf[16];

    sprintf(ansi_buf, ANSI_SGR, fg, bg);
    serial_puts(ansi_buf);
}

void serial_irq() {
    uint8_t isr = inb(COM1 + 2);
    if(isr & 0x01) return; // Stop if no interrupt is pending (Spurious IRQ)

    uint8_t vec = (isr >> 1) & 0x07;
    uint8_t data;

    switch(vec) {
        case 0: // Modem status
            break;
        case 1: // THR empty
            for(int i = 0; i < 16; i++) {
                if(rb_get(&rb_tx, &data)) break;
                outb(COM1, data);
            }
            break;
        case 2: // Data ready
            data = inb(COM1 + 0);
            break;
        case 3: // Line status
            inb(COM1 + 5); // Read LSR
            break;
        case 6: // RX timeout
            data = inb(COM1 + 0);
            break;
        case 7: // DMA end of transfer
            break;
    }
}