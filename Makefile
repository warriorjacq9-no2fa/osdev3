.PHONY: bios all clean

ROOT = $(CURDIR)

all: os.img kernel.dump
	qemu-system-i386 -hda $<

os.img: bios
	cp boot/bios/boot.img $@

bios:
	$(MAKE) -C boot/bios boot.img ROOT=$(ROOT)

kernel.dump: bios
	objdump -b binary -mi8086 --adjust-vma=0x7C00 -D boot/bios/boot.o > $@
	objdump -b binary -mi386 --adjust-vma=0x7E00 -D arch/x86/kernel.bin >> $@

clean:
	rm -f kernel.dump os.img
	$(MAKE) -C arch/x86 clean ROOT=$(ROOT)
	$(MAKE) -C boot/bios clean ROOT=$(ROOT)
