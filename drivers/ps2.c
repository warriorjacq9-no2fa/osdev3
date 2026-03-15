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
    0, 0, 0, 0
};

static char upper[] = {
    '"', '(', ')', '*', '+', '<', '_', '>', '?',
    ')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
    ':', ':', '<', '+'
};

static bool ext, shift, caps;

void ps2_irq() {
    uint8_t sc = inb(0x60);
    kevent_input_t evt;
    evt.type = KEVENT_KEY;
    if(ext); // TODO: PS/2 extended
    else {
        // No translation needed for non-extended SC1 keys since
        // they line up with the Kernel API keycodes
        evt.key.keycode = sc;
        evt.key.pressed = (sc & 0x80) ? false : true;
        kinput(&evt);
        
        // Send a character event as well
        evt.type = KEVENT_CHAR;
        char c = 0;
        switch(sc) {
            case 0xE0: ext = true; break;

            case 0x2A: // LSHIFT
            case 0x36: // RSHIFT
                shift = true;
                break;

            case 0xAA: // LSHIFT
            case 0xB6: // RSHIFT
                shift = false;
                break;
            
            case 0x3A: // CAPSLK
                caps = !caps;
                break;
            
            default:
                if(sc < sizeof(ps2_sc1) && ps2_sc1[sc]) {
                    c = ps2_sc1[sc];
                    if(shift) {
                        switch(c) {
                            case '[': c = '{'; break;
                            case '\\': c = '|'; break;
                            case ']': c = '}'; break;
                            case '`': c = '~'; break;

                            default:
                                if(c - '\'' < sizeof(upper)) c = upper[c - '\''];
                                break;
                        }
                    }
                    if(shift != caps && isalpha(c)) c -= ('a' - 'A');
                }
                break;
        }
        if(c) {
            evt.ch.character = c;
            kinput(&evt);
        }
    }
}