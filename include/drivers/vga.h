#ifndef D_VGA_H
#define D_VGA_H

#include <stdint.h>
#include <stdio.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_putc(char c);
void vga_puts(const char* s);
void vga_setcolor(uint8_t fg, uint8_t bg);

#endif