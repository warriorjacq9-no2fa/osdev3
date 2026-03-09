#include <drivers/vga.h>
#include <io.h>

static uint16_t* const VGA_MEM = (uint16_t*)0xB8000;

static uint8_t vga_row = 0, vga_col = 0;

void vga_clear(uint8_t x, uint8_t y, uint8_t len) {
    for(int i = 0; i < len; i++)
        VGA_MEM[y * VGA_WIDTH + i + x] = 
            ' ' | ((FGC_GRAY | BGC_BLACK) << 8);
}

void vga_scroll() {
    int len = 2 * ((VGA_HEIGHT - 1) * VGA_WIDTH);

    // Memmove inline
    char *d = (char*)VGA_MEM;
    const char *s = (const char*)(VGA_MEM + VGA_WIDTH);
    while(len--) *d++ = *s++;

    vga_clear(0, VGA_HEIGHT - 1, VGA_WIDTH);
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

void vga_putc(char c, uint8_t color) {
    if(c == '\n') {
        vga_clear(vga_col, vga_row, VGA_WIDTH - vga_col);
        vga_row++;
        vga_col = 0;
    } else if(c == '\r') {
        vga_col = 0;
    } else if(c == '\t') {
        vga_clear(vga_col, vga_row, 8);
        vga_col += 8;
    }
    else
        VGA_MEM[vga_row * VGA_WIDTH + vga_col] = c | (color << 8);
    
    if(++vga_col >= VGA_WIDTH) {
        vga_col = 0;
        if(++vga_row >= VGA_HEIGHT) {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
    }
    vga_set_cursor(vga_col, vga_row);
}

void vga_puts(const char* s, uint8_t color) {
    while(*s) {
        vga_putc(*s, color);
        s++;
    }
}
