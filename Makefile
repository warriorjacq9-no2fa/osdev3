# Increase this if size-related issues pop up
KERNEL_SIZE_KB = 32

CC = i386-elf-gcc
CFLAGS ?= \
-Iinclude \
-Iarch/x86/include \
-ffreestanding -fno-stack-protector \
-fpic \
-Os -march=i386 -m32 -flto \
-Wall -Werror \
-DKSIZE=$(KERNEL_SIZE_KB) -fno-delete-null-pointer-checks

AS = i386-elf-as
AFLAGS ?= --32 -march=i386

LDFLAGS ?= -nostdlib -no-pie -Wl,-Map=kernel.map -flto
LIBS ?= -lgcc

OBJS = \
arch/x86/boot.o \
arch/x86/arch.o \
arch/x86/interrupts.o \
arch/x86/io.o \
arch/x86/mm.o \
drivers/pic.o \
drivers/pit.o \
drivers/ps2.o \
drivers/serial.o \
drivers/vga.o \
kernel/kernel.o \
kernel/kevent.o \
kernel/klog.o \
kernel/kmalloc.o \
lib/ringbuffer/ringbuffer.o \
lib/stdio/printf.o \
lib/stdio/putc.o \
lib/stdio/puts.o \
lib/stdio/setcolor.o \
lib/string/memcpy.o \
lib/string/memmove.o \
lib/string/memset.o \
lib/string/strcat.o \
lib/string/strchr.o \
lib/string/strcpy.o \
lib/string/strlen.o \

HEADERS = \
arch/x86/include/arch.h \
arch/x86/include/io.h \
arch/x86/include/mm.h \
arch/x86/include/stddef.h \
arch/x86/interrupts.h \
include/drivers/pic.h \
include/drivers/pit.h \
include/drivers/ps2.h \
include/drivers/serial.h \
include/drivers/vga.h \
include/kernel/kevent.h \
include/kernel/klog.h \
include/kernel/kmalloc.h \
include/kernel/ringbuffer.h \
include/ansi.h \
include/stdio.h \
include/string.h

BIOS_OBJS = \
boot/bios/boot.o

.PHONY: clean test bios

os.img: $(BIOS_OBJS) kernel.bin
	cat $^ > $@

kernel.bin: arch/x86/linker.ld $(OBJS)
	$(CC) $(LDFLAGS) -T $^ $(LIBS) -o $(basename $@).elf
	i386-elf-objcopy -O binary $(basename $@).elf $(basename $@).obj
	cp $(basename $@).obj $@
	truncate -s 32K $@

test: os.img kernel.dump
	qemu-system-i386 -D qemu.log -d int \
		--no-reboot --no-shutdown \
		-hda $< \
		-nographic -serial mon:stdio

kernel.dump: os.img
	objdump -b binary -mi8086 --adjust-vma=0x7C00 -D $(BIOS_OBJS) > $@
	i386-elf-objdump -D kernel.elf >> $@

%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

%.o: %.asm
	nasm -dKSIZE=$(KERNEL_SIZE_KB) $< -fbin -o $@ 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f kernel.dump os.img $(OBJS) kernel.bin kernel.elf kernel.obj kernel.map boot/bios/boot.o qemu.log
