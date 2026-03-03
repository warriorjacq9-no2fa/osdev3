# This Makefile builds the boot image for a BIOS system. It needs a binary kernel image.
# The BIOS image uses NASM for flat-binary and multi-bits simplicity
.PHONY: kernel clean

ROOT ?= ../..

boot.img: boot.o kernel
	cat boot.o $(ROOT)/kernel/kernel.bin > $@

kernel:
	$(MAKE) -C $(ROOT)/kernel kernel.bin

%.o: %.asm
	nasm $^ -fbin -o $@

clean:
	rm -f boot.o boot.img