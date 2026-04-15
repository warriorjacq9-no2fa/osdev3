CC = i386-elf-gcc
CFLAGS ?= \
-I../include -I../arch/x86/include \
-ffreestanding -fno-stack-protector \
-fno-pic \
-Os -march=i386 -m32 \
-Wall -Werror \
-fno-delete-null-pointer-checks -g

AS = i386-elf-as
AFLAGS ?= --32 -march=i386

LDFLAGS ?= -nostdlib -no-pie -Wl,-z,stack-size=16384 -g
LIBS ?= -lgcc

OBJS = \
arch/x86/kcall.o \
main.o

HEADERS = 

.PHONY: clean

main.dump: main.bin
	i386-elf-objdump -S $(basename $@).elf > $@

main.bin: ../arch/x86/ulink.ld $(OBJS)
	$(CC) $(LDFLAGS) -T $^ $(LIBS) -o $(basename $@).elf
	i386-elf-objcopy -O binary $(basename $@).elf $(basename $@).obj
	cp $(basename $@).obj $@
	truncate -s 1K $@

%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.dump $(OBJS) main.bin main.elf main.obj
