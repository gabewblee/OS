all:
	nasm -f bin boot.asm -o boot.bin
	nasm -f bin pboot.asm -o pboot.bin
	dd if=/dev/zero of=disk.img bs=512 count=3000
	dd if=boot.bin of=disk.img bs=512 conv=notrunc
	dd if=pboot.bin of=disk.img bs=512 seek=1 conv=notrunc

run: all
	qemu-system-i386 -drive format=raw,file=disk.img

clean:
	rm boot.bin pboot.bin disk.img