#ifndef D_VGA_H
#define D_VGA_H

#include <stdint.h>

#define WIDTH 80
#define HEIGHT 25

enum fg_color {
    FGC_BLACK,
    FGC_BLUE,
    FGC_GREEN,
    FGC_CYAN,
    FGC_RED,
    FGC_MAGENTA,
    FGC_BROWN,
    FGC_GRAY,
    FGC_LIGHT
};

enum bg_color {
    BGC_BLACK,
    BGC_BLUE = 0x10,
    BGC_GREEN = 0x20,
    BGC_CYAN = 0x30,
    BGC_RED = 0x40,
    BGC_MAGENTA = 0x50,
    BGC_BROWN = 0x60,
    BGC_GRAY = 0x70,
    BGC_LIGHT = 0x80
};

void putc(char c, uint8_t color);
void puts(const char* s, uint8_t color);

#endif