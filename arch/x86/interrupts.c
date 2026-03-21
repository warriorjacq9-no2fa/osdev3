#include <stdint.h>
#include <stdio.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <drivers/pic.h>
#include <drivers/serial.h>
#include <drivers/ps2.h>
#include <arch.h>
#include <mm.h>
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
    registers_t r;
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

static char exceptions[][4] = {
    "#DE",
    "#DB",
    "#NI",
    "#BP",
    "#OF",
    "#BR",
    "#UD",
    "#NM",
    "#DF",
    "#CS",
    "#TS",
    "#NP",
    "#SS",
    "#GP",
    "#PF",
    "#RS",
    "#MF",
    "#AC",
    "#MC",
    "#XM",
    "#VE",
    "#CP",
    "#RS",
    "#RS",
    "#RS",
    "#RS",
    "#RS",
    "#RS",
    "#HV",
    "#VC",
    "#SX",
    "#RS",
};

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
    kprintf(LOG_INFO, "x86", "Loaded IDT with %u entries at %p\r\n", sizeof(idt) / sizeof(idt_entry_t), idt);
}

void exception_handler(iframe_t iframe) {
    if(iframe.vector > 31) {
        switch(iframe.vector - 32) {
            case 0: // Timer IRQ
                kevent_proc();
                break;
            case 1: // Keyboard IRQ
                ps2_irq();
                break;
            case 4: // COM1
                serial_irq();
                break;
        }
        pic_eoi(iframe.vector - 32);
    } else {
        if(iframe.vector == 0xE)
            if(!page_fault(get_cr2(), iframe.err_code)) return;
        kprintf(LOG_ERR, "x86", "Recieved exception %s\r\n", exceptions[iframe.vector]);
        kprintf(LOG_ERR, "x86",
            "Stack frame:\r\n\
            \tEDI: %08X ESI: %08X EBP: %08X ESP: %08X\r\n\
            \tEBX: %08X EDX: %08X ECX: %08X EAX: %08X\r\n\
            \tVEC: %08X ECODE: %08X EIP: %08X CS: %08X\r\n\
            \tEFLAGS: %08X\r\n",
            iframe.r.edi, iframe.r.esi, iframe.r.ebp, iframe.r.esp,
            iframe.r.ebx, iframe.r.edx, iframe.r.ecx, iframe.r.eax,
            iframe.vector, iframe.err_code, iframe.eip, iframe.cs,
            iframe.eflags
        );
        asm volatile("hlt");
    }
}