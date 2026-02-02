org 0
bits 16

; Set cs register to 0x07C0
jmp 0x07C0:start

start:
    cli
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Define drive variable
    mov [drive], dl

    ; Rewrite boot sector to [0x0060:0x0000]
    cld
    ; Copy 512 bytes from 0x07C0:0x0000 to 0x0060:0x0000 cx times
    mov si, 0x0000
    mov ax, 0x0060
    mov es, ax
    xor di, di
    mov cx, 512
    rep movsb
    jmp 0x0060:relocated

relocated:
    ; Reset ds register to 0x0060 so relocated labels work
    mov ax, cs
    mov ds, ax
    mov es, ax
    xor ax, ax
    mov ss, ax
    mov sp, 0x0600

    mov si, loading_msg
    call print

    ; Read partition boot sector using CHS
    xor ax, ax
    mov es, ax
    mov bx, 0x7C00
    mov ah, 0x02
    ; Number of sectors to read
    mov al, 0x01
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [drive]
    int 0x13

    ; Check if disk read was successful
    jc disk_read_failed

    ; Successful load of second stage bootloader
    mov si, loaded_msg
    call print

    sti
    ; Jump to loaded second stage bootloader
    jmp 0x0000:0x7C00

disk_read_failed:
    mov si, error_msg
    call print
    cli

.halt:
    hlt
    jmp .halt

print:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print

.done:
    ret

drive db 0
loading_msg db "Loading second stage bootloader...", 0x0D, 0x0A, 0
loaded_msg db "Loaded second stage bootloader", 0x0D, 0x0A, 0
error_msg db "Error: disk read failed", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55