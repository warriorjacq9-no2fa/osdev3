#ifndef D_SERIAL_H
#define D_SERIAL_H

#include <stdint.h>

#define COM1 0x3F8

typedef void (*s_callback_t)(uint8_t);

void serial_putc(char c);
void serial_puts(const char* s);

#endif