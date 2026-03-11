#ifndef D_PS2_H
#define D_PS2_H

#include <kernel/kinput.h>

#define KBD_CAPS    0xF0
#define KBD_NLOCK   0xF1
#define KBD_SCRLOCK 0xF2

#define PS2_RELEASE 0x80

static char ps2_sc1[] = {
    0,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    KI_LCTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KI_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KI_RSHIFT,
    '*', KI_LALT, ' ', KBD_CAPS,
    KI_F1, KI_F2, KI_F3, KI_F4, KI_F5, KI_F6, KI_F7, KI_F8, KI_F9, KI_F10,
    KBD_NLOCK, KBD_SCRLOCK,
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3', '0', '.',
    0,0,0, KI_F11, KI_F12
};

#endif