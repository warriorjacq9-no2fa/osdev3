#ifndef KINPUT_H
#define KINPUT_H

#define KI_RELEASE 0x40

enum kinput_control {
    KI_LCTRL = 0x80,
    KI_RCTRL,
    KI_LSHIFT,
    KI_RSHIFT,
    KI_LALT,
    KI_RALT,
    KI_F1,
    KI_F2,
    KI_F3,
    KI_F4,
    KI_F5,
    KI_F6,
    KI_F7,
    KI_F8,
    KI_F9,
    KI_F10,
    KI_F11,
    KI_F12,
    KI_WIN,
    KI_ESC,
    KI_UP,
    KI_LEFT,
    KI_RIGHT,
    KI_DOWN
};

void kinput(char c);

#endif