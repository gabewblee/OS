AS = nasm
CC ?= i686-elf-gcc
LD ?= i686-elf-ld
OBJCOPY ?= i686-elf-objcopy

CFLAGS = -ffreestanding -m32 -fno-pic -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib -m elf_i386

all: fboot.bin sboot.bin kernel.bin disk.img

fboot.bin: fboot.asm
	$(AS) -f bin $< -o $@

sboot.bin: sboot.asm
	$(AS) -f bin $< -o $@

entry.o: entry.asm
	$(AS) -f elf32 $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: entry.o kernel.o linker.ld
	$(LD) $(LDFLAGS) -o $@ entry.o kernel.o

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

disk.img: fboot.bin sboot.bin kernel.bin
	dd if=/dev/zero of=disk.img bs=512 count=3000
	dd if=fboot.bin of=disk.img bs=512 conv=notrunc
	dd if=sboot.bin of=disk.img bs=512 seek=1 conv=notrunc
	dd if=kernel.bin of=disk.img bs=512 seek=3 conv=notrunc

run: all
	qemu-system-i386 -drive format=raw,file=disk.img

clean:
	rm -f fboot.bin sboot.bin entry.o kernel.o kernel.elf kernel.bin disk.img