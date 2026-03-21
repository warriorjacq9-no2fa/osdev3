#ifndef ARCH_H
#define ARCH_H

#define KSTACK_BASE 0xC0010000

typedef struct registers {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} registers_t;

void sti();
void cli();
uint32_t get_cr0();
void set_cr0(uint32_t val);
uint32_t get_cr2();
uint32_t get_cr3();
void set_cr3(uint32_t val);
void arch_init();

#endif