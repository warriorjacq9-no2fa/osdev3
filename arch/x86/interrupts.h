#ifndef A_INTERRUPTS_H
#define A_INTERRUPTS_H

#include <stdint.h>

void isr_init();
void idt_add(uint8_t i, uint32_t isr, uint8_t flags);

#endif