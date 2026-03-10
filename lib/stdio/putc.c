#include <string.h>

void putc(char c) {
#if __has_include(<drivers/vga.h>)
#  include <drivers/vga.h>
    vga_putc(c);
#endif
#if __has_include(<drivers/serial.h>)
#  include <drivers/serial.h>
    serial_putc(c);
#endif
}