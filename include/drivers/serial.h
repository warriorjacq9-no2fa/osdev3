#ifndef D_SERIAL_H
#define D_SERIAL_H

#include <stdint.h>

#define COM1 0x3F8

void serial_putc(char c);
void serial_puts(const char* s);

#endif