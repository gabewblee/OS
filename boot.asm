bits 16
org 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    call print
    jmp $

print:
    mov al, 'H'
    mov ah, 0x0E
    int 0x10
    mov al, 'i'
    mov ah, 0x0E
    int 0x10
    ret

times 510-($-$$) db 0
dw 0xAA55