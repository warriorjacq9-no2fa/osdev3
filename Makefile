CC ?= gcc
CFLAGS ?= \
-Iinclude \
-Iarch/x86/include \
-ffreestanding -fno-stack-protector \
-fno-pic \
-O2 -march=i386 -m32 \
-Wall -Werror

AS ?= as
AFLAGS ?= --32 -march=i386

LDFLAGS ?= -nostdlib -no-pie -lgcc

OBJS = \
arch/x86/boot.o \
arch/x86/io.o \
drivers/serial.o \
drivers/vga.o \
kernel/kernel.o \
kernel/klog.o \
lib/stdio/printf.o \
lib/stdio/putc.o \
lib/stdio/puts.o \
lib/string/memset.o \
lib/string/strcat.o \
lib/string/strcpy.o \
lib/string/strlen.o \

HEADERS = \
include/drivers/serial.h \
include/drivers/vga.h \
include/kernel/klog.h \
include/ansi.h \
include/stdio.h \
include/string.h

BIOS_OBJS = \
boot/bios/boot.o

.PHONY: clean test bios

os.img: $(BIOS_OBJS) kernel.bin
	cat $^ > $@

kernel.bin: arch/x86/linker.ld $(OBJS)
	$(CC) $(LDFLAGS) -T $^ -o $@
	truncate -s 32K $@

test: os.img kernel.dump
	qemu-system-i386 -hda $<

kernel.dump: os.img
	objdump -b binary -mi8086 --adjust-vma=0x7C00 -D $(BIOS_OBJS) > $@
	objdump -b binary -mi386 --adjust-vma=0x7E00 -D kernel.bin >> $@

%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

%.o: %.asm
	nasm $< -fbin -o $@ 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f kernel.dump os.img $(OBJS) kernel.bin boot/bios/boot.o
