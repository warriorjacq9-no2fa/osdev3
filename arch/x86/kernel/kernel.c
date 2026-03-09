#include <drivers/vga.h>
#include <drivers/serial.h>

void kmain() {
    vga_puts("Hello world!");
    serial_puts("Hello world!");
}