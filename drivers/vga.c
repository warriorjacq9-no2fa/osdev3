#include <string.h>
#include <drivers/vga.h>
#include <io.h>

static uint16_t* const VGA_MEM = (uint16_t*)0xB8000;

static uint8_t vga_row = 0, vga_col = 0;
static uint8_t vga_color = FG_GRAY | (BG_BLACK << 4);

void vga_clear(uint8_t x, uint8_t y, uint8_t len) {
    for(int i = 0; i < len; i++)
        VGA_MEM[y * VGA_WIDTH + i + x] = 
            ' ' | ((FG_GRAY | BG_BLACK) << 8);
}

void vga_scroll() {
    memmove(
        &VGA_MEM[0],
        &VGA_MEM[VGA_WIDTH],
        (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t)
    );

    memset(
        &VGA_MEM[(VGA_HEIGHT - 1) * VGA_WIDTH],
        0,
        VGA_WIDTH * sizeof(uint16_t)
    );
}

static inline void vga_set_cursor(uint8_t x, uint8_t y) {
    vga_col = x;
    vga_row = y;

    uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void vga_putc(char c) {
    if(c == '\n') {
        vga_col = 0;
        if(++vga_row >= VGA_HEIGHT) {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
    } else if(c == '\r') {
        vga_col = 0;
    } else if(c == '\t') {
        vga_clear(vga_col, vga_row, 4);
        vga_col += 4;
        if(vga_col >= VGA_WIDTH) vga_col = VGA_WIDTH - 1;
    } else if(c) {
        VGA_MEM[vga_row * VGA_WIDTH + vga_col] = c | (vga_color << 8);
    
        if(++vga_col >= VGA_WIDTH) {
            vga_col = 0;
            if(++vga_row >= VGA_HEIGHT) {
                vga_row = VGA_HEIGHT - 1;
                vga_scroll();
            }
        }
    }
    vga_set_cursor(vga_col, vga_row);
}

void vga_puts(const char* s) {
    while(*s) {
        vga_putc(*s);
        s++;
    }
}

void vga_setcolor(uint8_t fg, uint8_t bg) {
    vga_color = fg | (bg << 4);
}