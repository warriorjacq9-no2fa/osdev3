#include <string.h>

void puts(const char* s) {
#if __has_include(<drivers/vga.h>)
#  include <drivers/vga.h>
    vga_puts(s);
#endif
#if __has_include(<drivers/serial.h>)
#  include <drivers/serial.h>
    serial_puts(s);
#endif
}