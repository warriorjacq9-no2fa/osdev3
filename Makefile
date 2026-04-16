# Increase this if size-related issues pop up
KERNEL_SIZE_KB = 32

CC = i386-elf-gcc
CFLAGS ?= \
-Iinclude \
-Iarch/x86/include \
-ffreestanding -fno-stack-protector \
-fpic \
-Os -march=i386 -m32 \
-Wall -Werror \
-DKSIZE=$(KERNEL_SIZE_KB) -fno-delete-null-pointer-checks -g

AS = i386-elf-as
AFLAGS ?= --32 -march=i386

LDFLAGS ?= -nostdlib -no-pie -Wl,-z,stack-size=16384 -Wl,-Map=kernel.map -g
LIBS ?= -lgcc

OBJS = \
arch/x86/boot.o \
arch/x86/kthread.o \
arch/x86/kt_ctx.o \
arch/x86/arch.o \
arch/x86/interrupts.o \
arch/x86/io.o \
arch/x86/mm.o \
drivers/ata.o \
drivers/pic.o \
drivers/pit.o \
drivers/ps2.o \
drivers/serial.o \
drivers/vga.o \
kernel/kcall.o \
kernel/kernel.o \
kernel/kevent.o \
kernel/klog.o \
kernel/kmalloc.o \
kernel/kthread.o \
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
arch/x86/include/ctx.h \
arch/x86/include/io.h \
arch/x86/include/mm.h \
arch/x86/include/stddef.h \
arch/x86/interrupts.h \
include/drivers/ata.h \
include/drivers/pic.h \
include/drivers/pit.h \
include/drivers/ps2.h \
include/drivers/serial.h \
include/drivers/vga.h \
include/kernel/kcall.h \
include/kernel/kevent.h \
include/kernel/klog.h \
include/kernel/kmalloc.h \
include/kernel/kthread.h \
include/kernel/ringbuffer.h \
include/ansi.h \
include/stdio.h \
include/string.h

BIOS_OBJS = \
boot/bios/boot.o

.PHONY: clean test bios usr

os.img: $(BIOS_OBJS) kernel.bin usr
	cat $(BIOS_OBJS) kernel.bin usr/main.bin > $@

usr:
	$(MAKE) -C usr

kernel.bin: arch/x86/linker.ld $(OBJS)
	$(CC) $(LDFLAGS) -T $^ $(LIBS) -o $(basename $@).elf
	i386-elf-objcopy -O binary $(basename $@).elf $(basename $@).obj
	cp $(basename $@).obj $@
	truncate -s 32K $@

test: os.img kernel.dump
	qemu-system-i386 -D qemu.log -d int \
		--no-reboot --no-shutdown \
		-hda $< -display curses \
		$(if $(DISPLAY),,-nographic -serial mon:stdio)

debug: os.img kernel.dump
	qemu-system-i386 -D qemu.log -d int \
		--no-reboot --no-shutdown \
		-hda $< \
		$(if $(DISPLAY),,-nographic -serial mon:stdio) \
		-s -S

kernel.dump: $(BIOS_OBJS) kernel.bin
	objdump -b binary -mi8086 --adjust-vma=0x7C00 -D $(BIOS_OBJS) > $@
	i386-elf-objdump -S kernel.elf >> $@

%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

%.o: %.asm
	nasm -dKSIZE=$(KERNEL_SIZE_KB) $< -fbin -o $@ 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C usr clean
	rm -f kernel.dump os.img $(OBJS) kernel.bin kernel.elf kernel.obj kernel.map boot/bios/boot.o qemu.log
