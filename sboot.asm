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

    sti

    call enable_a20
    jc a20_failed

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

	sti
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

a20_failed:
    cli

.hang:
    hlt
    jmp .hang

drive:
    db 0