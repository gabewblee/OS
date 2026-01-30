org 0x7C00
bits 16

start:
    cld

    cli
    ; Initialize the ds, es, and ss registers to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    add ax, 0x9000
    mov ss, ax
    ; Bottom of stack at 0x9000:0xF000
    mov sp, 0xF000

    ; Define drive variable
    mov [drive], dl

    call enable_a20
    jc a20_failed

	; Load the Kernel
	call load_kernel

    ; Disable NMI
    in al, 0x70
    or al, 0x80
    out 0x70, al
    in al, 0x71

    ; Enter protected mode
    ; Load GDT register with start address of Global Descriptor Table
    lgdt [gdtr]
    mov eax, cr0
    ; Set PE (Protection Enable) bit in CR0 (Control Register 0)
    or al, 1
    mov cr0, eax

    ; Make cs register hold the newly defined selector
    jmp 08h:protected

load_kernel:
	; Try LBA read (INT 13h Extensions)
	mov word [dap + 2], kernel_num_sectors
	mov word [dap + 4], 0x0000
	mov word [dap + 6], 0x1000
	mov dword [dap + 8], 3
	mov dword [dap + 12], 0
	mov si, dap
	mov ah, 0x42
	mov dl, [drive]
	int 0x13
	jnc .ok

	; Fallback to CHS read
	mov ah, 0x02        ; BIOS read sectors
	mov al, kernel_num_sectors
	mov ch, 0           ; cylinder 0
	mov cl, 4           ; sector 4
	mov dh, 0           ; head 0
	mov dl, [drive]     ; boot drive
	mov ax, 0x1000
	mov es, ax
	mov bx, 0x0000
	int 0x13
	jc disk_failed
.ok:
	ret

gdtr:
    ; gdtr size
    dw gdt_end - gdt - 1

    ; gdtr offset
    dd gdt

gdt:
    ; Null descriptor (Offset: 0x0000)
    dq 0x0000000000000000

    ; Kernel mode code segment (Offset: 0x0008)
    dq 0x00C09A000000FFFF

    ; Kernel mode data segment (Offset: 0x0010)
    dq 0x00C092000000FFFF

    ; User mode code segment (Offset: 0x0018)
    dq 0x00C0FA000000FFFF

    ; User mode data segment (Offset: 0x0020)
    dq 0x00C0F2000000FFFF

    ; Task state segment(?) (Offset: 0x0028)
    ; Needs to be set using C code due to usage of sizeof(TSS) for limit
    dq 0x0000000000000000

gdt_end:

; Checks whether the A20 line is currently enabled.
; Returns (in ax register):
;   0 - disabled
;   1 - enabled
get_a20_state:
	pushf
	push si
	push di
	push ds
	push es
	cli

	mov ax, 0x0000					;	0x0000:0x0500(0x00000500) -> ds:si
	mov ds, ax
	mov si, 0x0500

	not ax							;	0xffff:0x0510(0x00100500) -> es:di
	mov es, ax
	mov di, 0x0510

	mov al, [ds:si]					;	save old values
	mov byte [.BufferBelowMB], al
	mov al, [es:di]
	mov byte [.BufferOverMB], al

	mov ah, 1						;	check byte [0x00100500] == byte [0x0500]
	mov byte [ds:si], 0
	mov byte [es:di], 1
	mov al, [ds:si]
	cmp al, [es:di]
	jne .exit
	dec ah

.exit:
	mov al, [.BufferBelowMB]
	mov [ds:si], al
	mov al, [.BufferOverMB]
	mov [es:di], al
	shr ax, 8
	pop es
	pop ds
	pop di
	pop si
	popf
	ret
	
	.BufferBelowMB:	db 0
	.BufferOverMB	db 0

; Ask BIOS which A20 mechanisms exist
;
; Sets (in ax register):
;   Bit 0 - keyboard controller supported
;   Bit 1 - fast gate supported
;   CF    - does not support
query_a20_support:
	push bx
	clc

	mov ax, 0x2403
	int 0x15
	jc .error

	test ah, ah
	jnz .error

	mov ax, bx
	pop bx
	ret

.error:
	stc
	pop bx
	ret

; Enable A20 using the 8042 keyboard controller
enable_a20_keyboard_controller:
	pushf
	cli

	call .wait_io1
	mov al, 0xad
	out 0x64, al
	
	call .wait_io1
	mov al, 0xd0
	out 0x64, al
	
	call .wait_io2
	in al, 0x60
	push ax
	
	call .wait_io1
	mov al, 0xd1
	out 0x64, al
	
	call .wait_io1
	pop ax
	or al, 2
	out 0x60, al
	
	call .wait_io1
	mov al, 0xae
	out 0x64, al

	popf
	ret

; Waits until the keyboard controller input buffer is empty
.wait_io1:
	in al, 0x64
	test al, 2
	jnz .wait_io1
	ret

; Waits until the keyboard controller output buffer is full
.wait_io2:
	in al, 0x64
	test al, 1
	jz .wait_io2
	ret

; Enables the A20 line
enable_a20:
	clc									;	clear cf
	pusha
	mov bh, 0							;	clear bh

	call get_a20_state
	jc .fast_gate

	test ax, ax
	jnz .done

	call query_a20_support
	mov bl, al
	test bl, 1							;	enable A20 using keyboard controller
	jnz .keybord_controller

	test bl, 2							;	enable A20 using fast A20 gate
	jnz .fast_gate

; Attempts to enable A20 via BIOS interrupt INT 15h
.bios_int:
	mov ax, 0x2401
	int 0x15
	jc .fast_gate
	test ah, ah
	jnz .failed
	call get_a20_state
	test ax, ax
	jnz .done

; Attempts to enable A20 using port 0x92
.fast_gate:
	in al, 0x92
	test al, 2
	jnz .done

	or al, 2
	and al, 0xfe
	out 0x92, al

	call get_a20_state
	test ax, ax
	jnz .done

	test bh, bh							;	test if there was an attempt using the keyboard controller
	jnz .failed

; Fallback path that calls enable_a20_keyboard_controller
.keybord_controller:
	call enable_a20_keyboard_controller
	call get_a20_state
	test ax, ax
	jnz .done

	mov bh, 1							;	flag enable attempt with keyboard controller

	test bl, 2
	jnz .fast_gate
	jmp .failed

; Sets carry flag to indicate A20 could not be enabled
.failed:
	stc

; Successful exit
.done:
	popa
	ret

; Hang if the enabling the A20 line fails
a20_failed:
    hlt
    jmp a20_failed

disk_failed:
	hlt
	jmp disk_failed

drive:
	db 0

dap:
	db 0x10
	db 0x00
	dw 0
	dw 0
	dw 0
	dd 0
	dd 0

kernel_num_sectors equ 8
kernel_num_bytes   equ (kernel_num_sectors * 512)

entry equ 0x00100000

bits 32
protected:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x9FC00
	cld

	mov esi, 0x00010000
	mov edi, 0x00100000
	mov ecx, kernel_num_bytes / 4
	rep movsd

	mov eax, entry
	jmp eax