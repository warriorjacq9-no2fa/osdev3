#ifndef ARCH_H
#define ARCH_H

void sti();
void cli();
uint32_t get_cr0();
void set_cr0(uint32_t val);
uint32_t get_cr2();
uint32_t get_cr3();
void set_cr3(uint32_t val);
void arch_init();

#endif