#include <stdbool.h>
#include <drivers/ps2.h>
#include <io.h>
#include <ctype.h>

static char ps2_sc1[] = {
    0, KI_ESC,
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

static bool ext = false, shift = false, caps = false;

void ps2_irq() {
    uint8_t keycode = inb(0x60);
    unsigned char c = keycode < sizeof(ps2_sc1) ? ps2_sc1[keycode] : 0;
    if(keycode == 0xE0) {
        ext = 1;
    } else if(ext) {
        ext = 0;
    } else if(c) {
        switch(c) {
            case KBD_CAPS:
                caps = true;
                break;
            default:
                if(caps != shift) { // When only one is true
                    if(isalpha(c)) {
                        kinput(c - 0x20);
                    }
                } else kinput(c);
        }
    } else if(keycode & 0x80) {
        kinput(ps2_sc1[keycode - 0x80] | KI_RELEASE);
    }
}