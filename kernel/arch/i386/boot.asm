.code32
.section .text.start

.global _start
_start:
    mov $0x00010000, %esp
    call kmain

halt:
    hlt
    jmp halt

.section .text

.extern kmain
