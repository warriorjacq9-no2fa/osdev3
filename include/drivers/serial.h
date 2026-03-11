#ifndef D_SERIAL_H
#define D_SERIAL_H

#include <stdint.h>

#define COM1 0x3F8

void serial_init();
void serial_putc(char c);
void serial_puts(const char* s);
void serial_setcolor(uint8_t fg, uint8_t bg);
void serial_irq();

#endif