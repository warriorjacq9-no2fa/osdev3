#ifndef D_PIC_H
#define D_PIC_H

#include <stdint.h>

void pic_remap(uint8_t vec1, uint8_t vec2);
void pic_eoi(uint8_t irq);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);
void pic_setmask(uint8_t m_mask, uint8_t s_mask);

#endif