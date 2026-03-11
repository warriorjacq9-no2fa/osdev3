#include "interrupts.h"
#include <drivers/pic.h>
#include <drivers/serial.h>

void arch_init() {
    serial_init();
    isr_init();
    pic_remap(0x20, 0x28);
    pic_setmask(0xFF, 0xFF);
    pic_unmask(1);
    pic_unmask(4);
    asm volatile("sti");
}