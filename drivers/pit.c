#include <drivers/pit.h>
#include <io.h>

void pit_set(uint16_t div) {
    asm volatile("cli"); // TODO: assembly isolation
    outb(0x43, 0b00110110); // Set channel 0 both bytes, interrupt mode
    outb(0x40, div & 0xFF);
    outb(0x40, (div >> 8) & 0xFF);
    asm volatile("sti");
}