#include <drivers/pic.h>
#include <io.h>

void pic_remap(uint8_t vec1, uint8_t vec2) {
    outb(0x20, 0x11); // ICW1: indicate presence of ICW4
    iowait();
    outb(0xA0, 0x11);
    iowait();
    outb(0x21, vec1); // ICW2: set interrupt vector offsets
    iowait();
    outb(0xA1, vec2);
    iowait();
    outb(0x21, 0x2); // ICW3: set cascade identity
    iowait();
    outb(0xA1, 0x2);
    iowait();

    outb(0x21, 0x01); // ICW4: set optional features (8086 mode)
	iowait();
	outb(0xA1, 0x01);
	iowait();
}

void pic_eoi(uint8_t irq) {
    if(irq >= 8)
		outb(0xA0, 0x20);
	
	outb(0x20, 0x20);
}

void pic_setmask(uint8_t m_mask, uint8_t s_mask) {
    outb(0x21, m_mask);
    outb(0xA1, s_mask);
}

void pic_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}