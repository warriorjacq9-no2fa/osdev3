.PHONY: bios all clean

ROOT = $(CURDIR)

all: os.img
	qemu-system-i386 -hda $< -nographic

os.img: bios
	cp boot/bios/boot.img $@

bios:
	$(MAKE) -C boot/bios boot.img

kernel.dump: bios
	objdump -b binary -mi386 --adjust-vma=0x7E00 -D kernel/kernel.bin > $@

clean:
	rm -f kernel.dump os.img
	$(MAKE) -C kernel clean
	$(MAKE) -C boot/bios clean
