ENTRY(_start)
OUTPUT_ARCH(i386)
OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x7E00;
    .text : {
        *(.text.start)
        *(.text)
        . = 0xFDFF;
    }
}