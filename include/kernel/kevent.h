#ifndef KINPUT_H
#define KINPUT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum kevent_key {
    KEY_RESERVED,
    KEY_ESC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_ENTER,
    KEY_LEFTCTRL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_LEFTSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_RIGHTSHIFT,
    KEY_KPASTERISK,
    KEY_LEFTALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK,
    KEY_SCROLLLOCK,
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    KEY_KPMINUS,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,
    KEY_KPPLUS,
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP0,
    KEY_KPDOT,
    KEY_ZENKAKUHANKAKU = 85,
    KEY_102ND,
    KEY_F11,
    KEY_F12,
    KEY_R0,
    KEY_KATAKANA,
    KEY_HIRAGANA,
    KEY_HENKAN,
    KEY_KATAKANAHIRAGANA,
    KEY_MUHENKAN,
    KEY_KPJPCOMMA,
    KEY_KPENTER,
    KEY_RIGHTCTRL,
    KEY_KPSLASH,
    KEY_SYSRQ,
    KEY_RIGHTALT,
    KEY_LINEFEED,
    KEY_HOME,
    KEY_UP,
    KEY_PAGEUP,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_END,
    KEY_DOWN,
    KEY_PAGEDOWN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_MACRO,
    KEY_MUTE,
    KEY_VOLUMEDOWN,
    KEY_VOLUMEUP,
    KEY_POWER,
    KEY_KPEQUAL,
    KEY_KPPLUSMINUS,
    KEY_PAUSE,
    KEY_SCALE,
    KEY_KPCOMMA,
    KEY_HANGUEL,
    KEY_HANJA,
    KEY_YEN,
    KEY_LEFTMETA,
    KEY_RIGHTMETA,
    KEY_COMPOSE,
} kevent_key_t;

typedef enum {
    KEVENT_KEY,
    KEVENT_CHAR,
    KEVENT_REL,
    KEVENT_ABS,
    KEVENT_DEVICE
} kevent_itype_t;

typedef struct {
    kevent_itype_t type;

    union {
        struct {
            uint16_t keycode;
            bool pressed;
        } key;

        struct {
            char character;
        } ch;

        struct {
            int dx;
            int dy;
        } rel;

        struct {
            int x;
            int y;
        } abs;
    };

} kevent_input_t;

typedef void (*kevent_callback_t)(kevent_input_t*);

typedef struct kevent_consumer {
    kevent_callback_t callback;
    kevent_itype_t type;
} kevent_consumer_t;

void kevent_init(size_t buf_sz, size_t num_consumers);
int kevent_register(kevent_consumer_t consumer);
void kevent_proc();
int kinput(kevent_input_t *evt);

#endif