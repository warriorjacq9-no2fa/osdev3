.code32
.section .text.start

.global _start
_start:
    call mm_init
    lea high, %ecx
    jmp *%ecx
high:
    mov $0xC0017E00, %esp // 32 KiB stack

    lgdt gdt_descriptor // Load our GDT instead of the bootloader's
    jmp $0x08, $reload_cs   // far jump to reload CS

reload_cs:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    call kmain

halt:
    hlt
    jmp halt

.section .text

.global flush_tss
flush_tss:
    mov $0x28, %ax
    ltr %ax
    ret

.extern mm_init
.extern kmain

.macro isr_err number
_isr\number:
    push $\number
    jmp isr_stub
.endm

.macro isr_no_err number
_isr\number:
    push $0
    push $\number
    jmp isr_stub
.endm

.macro irq number
_isr\number:
    push $0
    push $\number
    jmp isr_stub
.endm

.macro isr_res number
_isr\number:
    hlt
.endm

.extern exception_handler

isr_no_err  0
isr_no_err  1
isr_no_err  2
isr_no_err  3
isr_no_err  4
isr_no_err  5
isr_no_err  6
isr_no_err  7
isr_err     8
isr_no_err  9
isr_err     10
isr_err     11
isr_err     12
isr_err     13
isr_err     14
isr_res     15
isr_no_err  16
isr_err     17
isr_no_err  18
isr_no_err  19
isr_no_err  20
isr_no_err  21
isr_res     22
isr_res     23
isr_res     24
isr_res     25
isr_res     26
isr_res     27
isr_no_err  28
isr_no_err  29
isr_err     30
isr_res     31
.extern _isr32
irq         33
irq         34
irq         35
irq         36
irq         37
irq         38
irq         39
irq         40
irq         41
irq         42
irq         43
irq         44
irq         45
irq         46
irq         47

isr_stub:
    cli
    pusha
    call exception_handler
    popa
    add $8, %esp
    iret

.extern irq_handler

.section .data

.global gdt_start
gdt_start:
    // Null descriptor
.quad   0x0
    // Code segment
.quad   0x00CF9B000000FFFF
    // Data segment
.quad   0x00CF92000000FFFF
    // Userspace code segment
.quad   0x00CFFB000000FFFF
    // Userspace data segment
.quad   0x00CFF2000000FFFF
    // TSS placeholder for C
.quad 0
gdt_end:

gdt_descriptor:
.word   gdt_end - gdt_start - 1
.long   gdt_start

.macro isr_label number
    .long _isr\number
.endm

.altmacro
.global isr_stubs
isr_stubs:
.set i,0
.rept 48
    isr_label %i
    .set i, i+1
.endr

