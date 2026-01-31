bits 32
global start
extern kmain

code_segment equ 0x08
data_segment equ 0x10

start:
    mov ax, data_segment
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov ebp, 0x00200000
	mov esp, ebp

    call kmain
	jmp $

.hang:
	jmp .hang