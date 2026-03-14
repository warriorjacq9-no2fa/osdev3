#include <stdbool.h>
#include <kernel/kevent.h>
#include <drivers/ps2.h>
#include <io.h>
#include <ctype.h>

static char ps2_sc1[] = {
    0, 0,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3', '0', '.',
    0,0,0, 0, 0
};

static bool ext = false;

void ps2_irq() {
    uint8_t keycode = inb(0x60);
    if(keycode == 0xE0) {
        ext = 1;
    } else if(ext) {
        ext = 0;
    } else if(keycode < KEY_F12 || keycode & 0x80) {
        kevent_input_t evt;
        evt.type = KEVENT_KEY;
        // No translation needed for non-extended SC1 keys since
        // they line up with the Kernel API keycodes
        evt.key.keycode = keycode;
        evt.key.pressed = (keycode & 0x80) ? false : true;
        kinput(&evt);

        // Send a character event as well
        evt.type = KEVENT_CHAR;
        evt.ch.character = ps2_sc1[keycode];
    }
}