GCC = i686-elf-gcc
LD = i686-elf-ld
AS = nasm

BOOT = src/boot
BUILD = build
DRIVERS = src/drivers
INTERRUPTS = src/interrupts
MEMORY = src/memory

CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdlib
LDFLAGS = -T src/boot/linker.ld --oformat binary
SIZE = 102

all: $(BUILD)/fboot.bin $(BUILD)/sboot.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$(BUILD)/kernel.img bs=512 count=$(SIZE)
	dd if=$(BUILD)/fboot.bin of=$(BUILD)/kernel.img conv=notrunc
	dd if=$(BUILD)/sboot.bin of=$(BUILD)/kernel.img bs=512 seek=1 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$(BUILD)/kernel.img bs=512 seek=2 conv=notrunc

$(BUILD)/fboot.bin: $(BOOT)/fboot.asm
	$(AS) -f bin $< -o $@

$(BUILD)/sboot.bin: $(BOOT)/sboot.asm
	$(AS) -f bin $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel.asm.o $(BUILD)/kernel.o $(BUILD)/vga.o $(BUILD)/idt.o $(BUILD)/isr.o $(BUILD)/falloc.o
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/kernel.asm.o: $(BOOT)/kernel.asm
	$(AS) -f elf32 $< -o $@

$(BUILD)/kernel.o: $(BOOT)/kernel.c
	$(GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/vga.o: $(DRIVERS)/vga.c
	$(GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/idt.o: $(INTERRUPTS)/idt.c
	$(GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/isr.o: $(INTERRUPTS)/isr.asm
	$(AS) -f elf32 $< -o $@

$(BUILD)/falloc.o: $(MEMORY)/falloc.c
	$(GCC) $(CFLAGS) -c $^ -o $@

run: all
	qemu-system-i386 -vga none -device isa-vga -drive format=raw,file=$(BUILD)/kernel.img

clean:
	rm -f $(BUILD)/*.bin $(BUILD)/*.o $(BUILD)/kernel.img

.PHONY: all run clean