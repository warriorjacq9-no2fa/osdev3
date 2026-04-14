#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>

#define KSTACK_BASE 0xC0010000

typedef struct registers {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} registers_t;

typedef struct tss_entry {
    uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} tss_entry_t;

void wait();
uint32_t get_cr0();
void set_cr0(uint32_t val);
uint32_t get_cr2();
uint32_t get_cr3();
void set_cr3(uint32_t val);
void lock();
void unlock();
void arch_init();
void usermode_init();
void kstack_update(uintptr_t sp);

#endif