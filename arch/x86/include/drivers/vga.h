#ifndef D_VGA_H
#define D_VGA_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum fg_color {
    VGA_FG_BLACK,
    VGA_FG_BLUE,
    VGA_FG_GREEN,
    VGA_FG_CYAN,
    VGA_FG_RED,
    VGA_FG_MAGENTA,
    VGA_FG_BROWN,
    VGA_FG_GRAY,
    VGA_FG_LIGHT
};

enum bg_color {
    VGA_BG_BLACK,
    VGA_BG_BLUE = 0x10,
    VGA_BG_GREEN = 0x20,
    VGA_BG_CYAN = 0x30,
    VGA_BG_RED = 0x40,
    VGA_BG_MAGENTA = 0x50,
    VGA_BG_BROWN = 0x60,
    VGA_BG_GRAY = 0x70,
    VGA_BG_LIGHT = 0x80
};

void vga_putc(char c);
void vga_puts(const char* s);

#endif