#ifndef D_PS2_H
#define D_PS2_H

#include <kernel/kinput.h>

#define KBD_CAPS    0xF0
#define KBD_NLOCK   0xF1
#define KBD_SCRLOCK 0xF2

#define PS2_RELEASE 0x80

void ps2_irq();

#endif