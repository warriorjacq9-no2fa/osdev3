#include <arch.h>
#include <kernel/kmalloc.h>
#include "interrupts.h"
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <drivers/serial.h>
#include <string.h>

void wait() {
    asm volatile("hlt");
}

uint32_t get_cr0() {
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

inline void set_cr0(uint32_t val) { 
    asm volatile("mov %0, %%cr0" :: "r"(val) : "memory");
}

uint32_t get_cr2() {
    uint32_t val;
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

uint32_t get_cr3() {
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

inline void set_cr3(uint32_t val) { 
    asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
}

inline void lock() {
    asm volatile("cli");
}

inline void unlock() {
    asm volatile("sti");
}

static tss_entry_t *tss;
extern uint8_t gdt_start[48];
extern void flush_tss(void);

void usermode_init() {
    tss = kmalloc(sizeof(tss_entry_t), 0);
    memset(tss, 0, sizeof(tss_entry_t));

    uint32_t addr = (uint32_t)tss;
    uint32_t size = sizeof(tss_entry_t) - 1;

    gdt_start[5 * 8 + 0] = size & 0xFF; // Limit 0
    gdt_start[5 * 8 + 1] = (size >> 8) & 0xFF; // Limit 1
    gdt_start[5 * 8 + 2] = addr & 0xFF; // Base 0
    gdt_start[5 * 8 + 3] = (addr >> 8) & 0xFF; // Base 1
    gdt_start[5 * 8 + 4] = (addr >> 16) & 0xFF; // Base 2
    gdt_start[5 * 8 + 5] = 0x89; // Access byte
    gdt_start[5 * 8 + 6] = (size >> 16) & 0x0F; // Flags, Limit 2
    gdt_start[5 * 8 + 7] = addr >> 24; // Base 3

    tss->ss0 = 0x10;
    tss->esp0 = 0xC0017E00;
    flush_tss();
}

void kstack_update(uintptr_t sp) {
    tss->esp0 = (uint32_t)sp;
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
    asm volatile("sti");
}
