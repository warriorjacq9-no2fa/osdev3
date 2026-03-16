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

uint32_t get_cr0() {
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

void set_cr0(uint32_t val) { 
    asm volatile("mov %0, %%cr0" :: "r"(val) : "memory");
}

uint32_t get_cr3() {
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

void set_cr3(uint32_t val) { 
    asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
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
