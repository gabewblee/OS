org 0x7C00
bits 16

start:
    cld

    cli
    ; Initialize the ds, es, and ss registers to 0x07C0
	xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

	; Enable A20 line
    call enable_a20

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

enable_a20:
	in al, 0x92
	test al, 2
	jnz after
	or al, 2
	and al, 0xFE
	out 0x92, al

after:
	ret

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
    ; Kernel mode code segment (Offset: 0x0008) - 4GB limit so kernel can access 0xB8000 (VGA)
    dq 0x00CF9A000000FFFF

gdt_kernel_data_segment:
    ; Kernel mode data segment (Offset: 0x0010) - 4GB limit so kernel can access 0xB8000 (VGA)
    dq 0x00CF92000000FFFF

gdt_user_code_segment:
    ; User mode code segment (Offset: 0x0018)
    dq 0x00CFFA000000FFFF

gdt_user_data_segment:
    ; User mode data segment (Offset: 0x0020)
    dq 0x00CFF2000000FFFF

gdt_task_state_segment:
    ; Task state segment(?) (Offset: 0x0028)
    ; Needs to be set using C code due to usage of sizeof(TSS) for limit
    dq 0x0000000000000000

gdt_end:

bits 32
protected:
	mov eax, 2
	mov ecx, kernel_num_sectors
	mov edi, 0x00100000
	call ata_lba_read
	jmp code_segment:0x00100000
	jmp $

kernel_num_sectors equ 100
code_segment equ gdt_kernel_code_segment - gdt_start
data_segment equ gdt_kernel_data_segment - gdt_start
times 512-($-$$) db 0