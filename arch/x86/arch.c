#include "interrupts.h"
#include <drivers/pic.h>

void arch_init() {
    isr_init();
    pic_remap(0x20, 0x28);
    pic_setmask(0xFF, 0xFF);
    pic_unmask(0);
    asm volatile("sti");
}