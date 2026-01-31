org 0x7C00
bits 16

code_segment equ gdt_kernel_code_segment - gdt_start
data_segment equ gdt_kernel_data_segment - gdt_start

start:
    cld

    cli
    ; Initialize the ds, es, and ss registers to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

	; Enable A20 line
    call enable_a20
    jc a20_failed

    ; Disable NMI
    in al, 0x70
    or al, 0x80
    out 0x70, al
    in al, 0x71

    ; Enter protected mode
    ; Load GDT register with start address of Global Descriptor Table
    lgdt [gdt_descriptor]
    mov eax, cr0
    ; Set PE (Protection Enable) bit in CR0 (Control Register 0)
    or al, 1
    mov cr0, eax

    ; Make cs register hold the newly defined selector
    jmp code_segment:protected

gdt_descriptor:
    ; gdt_descriptor size
    dw gdt_end - gdt_start - 1

    ; gdt_descriptor offset
    dd gdt_start

gdt_start:
gdt_null:
    ; Null descriptor (Offset: 0x0000)
    dq 0x0000000000000000

gdt_kernel_code_segment:
    ; Kernel mode code segment (Offset: 0x0008)
    dq 0x00C09A000000FFFF

gdt_kernel_data_segment:
    ; Kernel mode data segment (Offset: 0x0010)
    dq 0x00C092000000FFFF

gdt_user_code_segment:
    ; User mode code segment (Offset: 0x0018)
    dq 0x00C0FA000000FFFF

gdt_user_data_segment:
    ; User mode data segment (Offset: 0x0020)
    dq 0x00C0F2000000FFFF

gdt_task_state_segment:
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

kernel_num_sectors equ 100

bits 32
protected:
	mov eax, 2
	mov ecx, kernel_num_sectors
	mov edi, 0x00100000
	call ata_lba_read
	jmp code_segment:0x00100000

; Reads sectors from ATA disk using LBA addressing
ata_lba_read:
	mov ebx, eax
	shr eax, 24
	or eax, 0xE0
	mov dx, 0x1F6
	out dx, al

	mov eax, ecx
	mov dx, 0x1F2
	out dx, al

	mov eax, ebx
	mov dx, 0x1F3
	out dx, al

	mov dx, 0x1F4
	mov eax, ebx
	shr eax, 8
	out dx, al

	mov dx, 0x1F5
	mov eax, ebx
	shr eax, 16
	out dx, al

	mov dx, 0x1F7
	mov al, 0x20
	out dx, al

.next_sector:
	push ecx

.try_again:
	mov dx, 0x1F7
	in al, dx
	test al, 8
	jz .try_again

	mov ecx, 256
	mov dx, 0x1F0
	rep insw
	pop ecx
	loop .next_sector
	ret

times 512-($-$$) db 0