# This Makefile builds the boot image for a BIOS system. It needs a binary kernel image.
# The BIOS image uses NASM for flat-binary and multi-bits simplicity
.PHONY: kernel clean

boot.img: boot.o kernel
	cat boot.o $(ROOT)/arch/x86/kernel.bin > $@

kernel:
	$(MAKE) -C $(ROOT)/arch/x86 kernel.bin ROOT=$(ROOT)

%.o: %.asm
	nasm $^ -fbin -o $@

clean:
	rm -f boot.o boot.img