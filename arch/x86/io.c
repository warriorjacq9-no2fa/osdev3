#include <io.h>

inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

inline void iowait() {
    outb(0x80, 0);
}