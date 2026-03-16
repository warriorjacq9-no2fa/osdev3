ENTRY(_start)
OUTPUT_ARCH(i386)

SECTIONS
{
    . = 0x7E00;
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
}