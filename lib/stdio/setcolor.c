#include <string.h>
#include <ansi.h>

void setcolor(uint8_t fg, uint8_t bg) {
#if __has_include(<drivers/vga.h>)
#  include <drivers/vga.h>
    vga_setcolor(fg, bg);
#endif
#if __has_include(<drivers/serial.h>)
#  include <drivers/serial.h>
    char* s_fg, s_bg;
    switch(fg) {
        case FG_BLACK: s_fg = ANSI_FG_BLACK;
        case FG_BLUE: s_fg = ANSI_FG_BLUE;
        case FG_BROWN: s_fg = ANSI_FG_YELLOW;
        case FG_CYAN: s_fg = ANSI_FG_CYAN;
        case FG_GRAY: s_fg = ANSI_FG_DEFAULT;
        case FG_GREEN: s_fg = ANSI_FG_GREEN;
        case FG_MAGENTA: s_fg = ANSI_FG_MAGENTA;
        case FG_RED: s_fg = ANSI_FG_RED;
    }
    switch(bg) {
        case BG_BLACK: s_fg = ANSI_BG_BLACK;
        case BG_BLUE: s_fg = ANSI_BG_BLUE;
        case BG_BROWN: s_fg = ANSI_BG_YELLOW;
        case BG_CYAN: s_fg = ANSI_BG_CYAN;
        case BG_GRAY: s_fg = ANSI_BG_DEFAULT;
        case BG_GREEN: s_fg = ANSI_BG_GREEN;
        case BG_MAGENTA: s_fg = ANSI_BG_MAGENTA;
        case BG_RED: s_fg = ANSI_BG_RED;
    }

    serial_setcolor()
#endif
}