NASM = nasm
I686_ELF_GCC = i686-elf-gcc
I686_ELF_LD = i686-elf-ld
I686_ELF_OBJCOPY = i686-elf-objcopy
GCC = gcc

BOOT = src/boot
BUILD = build
DRIVERS = src/drivers
INTERRUPTS = src/interrupts
MEMORY = src/memory
TESTS = tests

CFLAGS = -ffreestanding -O0 -nostdlib -g
TCFLAGS = -DTEST -I$(TESTS) -Wall -Wextra -O0 -g
LDFLAGS = -T src/boot/linker.ld
SIZE = 102

all: $(BUILD)/fboot.bin $(BUILD)/sboot.bin $(BUILD)/kernel.bin $(BUILD)/kernel.elf
	dd if=/dev/zero of=$(BUILD)/kernel.img bs=512 count=$(SIZE)
	dd if=$(BUILD)/fboot.bin of=$(BUILD)/kernel.img conv=notrunc
	dd if=$(BUILD)/sboot.bin of=$(BUILD)/kernel.img bs=512 seek=1 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$(BUILD)/kernel.img bs=512 seek=2 conv=notrunc

$(BUILD)/fboot.bin: $(BOOT)/fboot.asm
	$(NASM) -f bin $< -o $@

$(BUILD)/sboot.bin: $(BOOT)/sboot.asm
	$(NASM) -f bin $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(I686_ELF_OBJCOPY) -O binary $< $@

$(BUILD)/kernel.elf: $(BUILD)/kernel.asm.o $(BUILD)/kernel.o $(BUILD)/vga.o $(BUILD)/idt.o $(BUILD)/isr.o $(BUILD)/pic.o $(BUILD)/falloc.o $(BUILD)/paging.o $(BUILD)/mmap.o
	$(I686_ELF_LD) -T src/boot/linker.ld $^ -o $@

$(BUILD)/kernel.asm.o: $(BOOT)/kernel.asm
	$(NASM) -f elf32 $< -o $@

$(BUILD)/kernel.o: $(BOOT)/kernel.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/vga.o: $(DRIVERS)/vga.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/idt.o: $(INTERRUPTS)/idt.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/isr.o: $(INTERRUPTS)/isr.asm
	$(NASM) -f elf32 $< -o $@

$(BUILD)/pic.o: $(INTERRUPTS)/pic.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/falloc.o: $(MEMORY)/falloc.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/paging.o: $(MEMORY)/paging.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

$(BUILD)/mmap.o: $(MEMORY)/mmap.c
	$(I686_ELF_GCC) $(CFLAGS) -c $^ -o $@

# Test executable
$(BUILD)/tests: $(BUILD)/test_runner.o $(BUILD)/test_falloc.o $(BUILD)/test_mmap.o $(BUILD)/falloc_host.o $(BUILD)/mmap_host.o
	$(GCC) $(TCFLAGS) $^ -o $@

$(BUILD)/test_runner.o: $(TESTS)/test_runner.c
	$(GCC) $(TCFLAGS) -c $< -o $@

$(BUILD)/test_falloc.o: $(TESTS)/test_falloc.c $(TESTS)/test_falloc.h
	$(GCC) $(TCFLAGS) -c $(TESTS)/test_falloc.c -o $@

$(BUILD)/test_mmap.o: $(TESTS)/test_mmap.c $(TESTS)/test_mmap.h
	$(GCC) $(TCFLAGS) -c $(TESTS)/test_mmap.c -o $@

$(BUILD)/falloc_host.o: $(MEMORY)/falloc.c
	$(GCC) $(TCFLAGS) -c $< -o $@

$(BUILD)/mmap_host.o: $(MEMORY)/mmap.c
	$(GCC) $(TCFLAGS) -c $< -o $@

tests: $(BUILD)/tests
	$(BUILD)/tests

run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD)/kernel.img

clean:
	rm -f $(BUILD)/*.bin $(BUILD)/*.o $(BUILD)/kernel.img $(BUILD)/kernel.elf $(BUILD)/tests

.PHONY: all run tests clean
