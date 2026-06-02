ENTRY(_start)
OUTPUT_ARCH(i386)

SECTIONS
{
    . = 0xC0007E00;
    .text : {
        *(.text.start)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .initcall ALIGN(4) : {
        __initcall_start = .;
        KEEP(*(.initcall.low))
        KEEP(*(.initcall.0))
        KEEP(*(.initcall.1))
        KEEP(*(.initcall.2))
        KEEP(*(.initcall.3))
        __initcall_end = .;
    }

    .bss (NOLOAD) : {
        *(COMMON)
        *(.bss)
    }

    __kheap_start = .;
}