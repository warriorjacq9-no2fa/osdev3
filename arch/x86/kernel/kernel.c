#include <drivers/vga.h>

void kmain() {
    vga_puts("Hello world!", FGC_GRAY | BGC_BLACK);
}