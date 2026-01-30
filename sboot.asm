org 0x7C00
bits 16

start:
    cld
    
    cli
    ; Initialize the ds, es, and ss registers to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    ; Initialize the sp register to boot sector
    mov sp, 0x7C00
    sti
