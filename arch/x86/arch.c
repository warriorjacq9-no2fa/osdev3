#include "interrupts.h"
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <drivers/serial.h>

void sti() {
    asm volatile("sti");
}

void cli() {
    asm volatile("cli");
}

void arch_init() {
    serial_init();
    isr_init();
    pic_remap(0x20, 0x28);
    pic_setmask(0xFF, 0xFF);
    pit_set(65535);
    pic_unmask(0);
    pic_unmask(1);
    pic_unmask(4);
    sti();
}
