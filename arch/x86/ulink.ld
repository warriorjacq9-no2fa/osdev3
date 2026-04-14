ENTRY(main)
OUTPUT_ARCH(i386)

SECTIONS
{
    . = 0x10000008;
    .text : {
        *(.text.startup)
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