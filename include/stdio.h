#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stdint.h>

enum fg_color {
    FG_BLACK,
    FG_BLUE,
    FG_GREEN,
    FG_CYAN,
    FG_RED,
    FG_MAGENTA,
    FG_YELLOW = 0xE,
    FG_GRAY = 7,
    FG_LIGHT
};

enum bg_color {
    BG_BLACK,
    BG_BLUE = 0x10,
    BG_GREEN = 0x20,
    BG_CYAN = 0x30,
    BG_RED = 0x40,
    BG_MAGENTA = 0x50,
    BG_YELLOW = 0xE0,
    BG_GRAY = 0x70,
    BG_LIGHT = 0x80
};

void puts(const char* s);
void putc(char c);
void setcolor(uint8_t fg, uint8_t bg);

int printf(const char* format, ...);
int vprintf(const char* format, va_list list);
int sprintf(char* str, const char* format, ...);
int vsprintf(char* str, const char* format, va_list list);

#endif