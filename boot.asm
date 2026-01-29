org 0x7C00
bits 16

start:
    cli
    ; Initialize the ds, es, and ss registers to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Define drive variable
    mov [drive], dl

    ; Initialize the sp register to boot sector
    mov sp, 0x7C00
    sti

    ; Rewrite boot sector to [0x0060:0x0000]
    cld
    mov si, 0x7C00
    mov ax, 0x0060
    mov es, ax
    xor di, di
    mov cx, 512
    rep movsb
    jmp 0x0060:relocated

relocated:
    ; Reset ds register to 0x0060 so drive label works
    mov ax, cs
    mov ds, ax

    ; Read partition boot sector
    mov ah, 0x02
    mov al, 0x01
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [drive]
    xor ax, ax
    mov es, ax
    mov bx, 0x7C00
    int 0x13

    jmp 0x0000:0x7C00

drive:
    db 0

times 510-($-$$) db 0
dw 0xAA55