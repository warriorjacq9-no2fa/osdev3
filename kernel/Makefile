CC ?= gcc
CFLAGS ?= \
-ffreestanding -fno-stack-protector \
-O2 -march=i386 -m32 \
-Wall -Werror

AS ?= as
AFLAGS ?= --32 -march=i386

LD ?= ld
LDFLAGS ?= -nostdlib

OBJS = \
arch/i386/boot.o \
kernel.o

.PHONY: clean

kernel.bin: arch/i386/linker.ld $(OBJS)
	$(LD) $(LDFLAGS) -T $^ -o $@

%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJS) kernel.bin