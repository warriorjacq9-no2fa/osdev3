CC ?= gcc
CFLAGS ?= \
-I$(ROOT)/arch/x86/include \
-ffreestanding -fno-stack-protector \
-fno-pic \
-O2 -march=i386 -m32 \
-Wall -Werror

AS ?= as
AFLAGS ?= --32 -march=i386

LD ?= ld
LDFLAGS ?= -nostdlib -no-pie

OBJS = \
boot.o \
io.o \
$(ROOT)/kernel/log.o \
drivers/vga.o \
drivers/serial.o \
kernel/kernel.o

HEADERS = \
$(ROOT)/include/ansi.h \
include/io.h \
include/drivers/vga.h \
include/drivers/serial.h

.PHONY: clean

kernel.bin: linker.ld $(OBJS)
	$(LD) $(LDFLAGS) -T $^ -o $@
	truncate -s 32K $@

%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) kernel.bin