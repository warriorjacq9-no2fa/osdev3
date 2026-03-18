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

    .bss (NOLOAD) : {
        *(COMMON)
        *(.bss)
    }

    __kheap_start = .;
}