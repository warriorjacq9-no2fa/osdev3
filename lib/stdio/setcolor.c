#include <string.h>
#include <ansi.h>
#include <stdio.h>

void setcolor(uint8_t fg, uint8_t bg) {
#if __has_include(<drivers/vga.h>)
#  include <drivers/vga.h>
    vga_setcolor(fg, bg);
#endif
#if __has_include(<drivers/serial.h>)
#  include <drivers/serial.h>
    uint8_t s_fg, s_bg;
    switch(fg) {
        case FG_BLACK: s_fg = ANSI_FG_BLACK; break;
        case FG_BLUE: s_fg = ANSI_FG_BLUE; break;
        case FG_YELLOW: s_fg = ANSI_FG_YELLOW; break;
        case FG_CYAN: s_fg = ANSI_FG_CYAN; break;
        case FG_GRAY: s_fg = ANSI_FG_DEFAULT; break;
        case FG_GREEN: s_fg = ANSI_FG_GREEN; break;
        case FG_MAGENTA: s_fg = ANSI_FG_MAGENTA; break;
        case FG_RED: s_fg = ANSI_FG_RED; break;
        default: s_fg = ANSI_FG_DEFAULT;
    }
    switch(bg) {
        case BG_BLACK: s_bg = ANSI_BG_BLACK; break;
        case BG_BLUE: s_bg = ANSI_BG_BLUE; break;
        case BG_YELLOW: s_bg = ANSI_BG_YELLOW; break;
        case BG_CYAN: s_bg = ANSI_BG_CYAN; break;
        case BG_GRAY: s_bg = ANSI_BG_DEFAULT; break;
        case BG_GREEN: s_bg = ANSI_BG_GREEN; break;
        case BG_MAGENTA: s_bg = ANSI_BG_MAGENTA; break;
        case BG_RED: s_bg = ANSI_BG_RED; break;
        default: s_bg = ANSI_FG_DEFAULT;
    }

    serial_setcolor(s_fg, s_bg);
#endif
}