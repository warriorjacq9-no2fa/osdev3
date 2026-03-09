#include <drivers/vga.h>

static uint16_t* const VGA_MEM = (uint16_t*)0xB8000;

static uint8_t vga_row = 0, vga_col = 0;

void vga_scroll() {
    int len = 2 * ((HEIGHT - 1) * WIDTH);

    // Memmove inline
    char *d = (char*)VGA_MEM;
    const char *s = (const char*)(VGA_MEM + WIDTH);
    while(len--) *d++ = *s++;

    for(int i = 0; i < WIDTH; i++)
        VGA_MEM[(HEIGHT - 1) * WIDTH + i] = 
            ' ' | ((FGC_GRAY | BGC_BLACK) << 8);
}

void putc(char c, uint8_t color) {
    VGA_MEM[vga_row * WIDTH + vga_col] = c | (color << 8);
    if(++vga_col == WIDTH) {
        vga_col = 0;
        if(++vga_row == HEIGHT) {
            vga_row = HEIGHT - 1;
            vga_scroll();
        }
    }
}

void puts(const char* s, uint8_t color) {
    while(*s) {
        putc(*s, color);
        s++;
    }
}
