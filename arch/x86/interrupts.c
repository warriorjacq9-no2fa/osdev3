#include <stdint.h>
#include <stdio.h>
#include <kernel/klog.h>
#include <drivers/pic.h>
#include <io.h>

#define GT_TASK     0x5
#define GT_INT16    0x6
#define GT_TRAP16   0x7
#define GT_INT32    0xE
#define GT_TRAP32   0xF

typedef struct idt_entry {
    uint16_t    offset_low;
    uint16_t    segment;
    uint8_t     res;
    uint8_t     flags;
    uint16_t    offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct iframe {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t vector;
    uint32_t err_code;
    uint32_t eip, cs;
    uint32_t eflags;
} iframe_t;

typedef struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

static idt_entry_t idt[48];
static idtr_t idt_r;

extern uint32_t isr_stubs[48];

void idt_add(uint8_t i, uint32_t isr, uint8_t flags) {
    idt[i].flags = flags | 0x80;
    idt[i].segment = 0x08; // Entry 1, GDT, ring 0
    idt[i].offset_low = isr & 0xFFFF;
    idt[i].offset_high = (isr >> 16) & 0xFFFF;
    idt[i].res = 0;
}

void isr_init() {
    for(int i = 0; i < 48; i++) {
        idt_add(i, isr_stubs[i], GT_INT32);
    }
    idt_r.base = (uint32_t)&idt[0];
    idt_r.limit = sizeof(idt) - 1;

    asm volatile("lidt %0" : : "m" (idt_r) : "memory");
    kprintf(LOG_INFO, "x86", "Loaded IDT at base %08x, limit %04x\r\n", idt_r.base, idt_r.limit);
}

void exception_handler(iframe_t iframe) {
    if(iframe.vector > 31) {
        switch(iframe.vector - 32) {
            case 0: // Timer IRQ
                putc('t');
                break;
        }
        pic_eoi(iframe.vector - 32);
    } else {
        kprintf(LOG_ERR, "x86", "Recieved exception %d\r\n", iframe.vector);
        asm volatile("hlt");
    }
}