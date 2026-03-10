#ifndef D_VGA_H
#define D_VGA_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum fg_color {
    FG_BLACK,
    FG_BLUE,
    FG_GREEN,
    FG_CYAN,
    FG_RED,
    FG_MAGENTA,
    FG_BROWN,
    FG_GRAY,
    FG_LIGHT
};

enum bg_color {
    BG_BLACK,
    BG_BLUE = 0x10,
    BG_GREEN = 0x20,
    BG_CYAN = 0x30,
    BG_RED = 0x40,
    BG_MAGENTA = 0x50,
    BG_BROWN = 0x60,
    BG_GRAY = 0x70,
    BG_LIGHT = 0x80
};

void vga_putc(char c);
void vga_puts(const char* s);
void vga_setcolor(uint8_t fg, uint8_t bg);

#endif