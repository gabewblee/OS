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
    ; Set up segment registers for protected mode
    mov ax, data_segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x00300000         ; Set up stack in protected mode
    
    ; Load the kernel
    call load_kernel
    
    ; Jump to kernel
    jmp code_segment:0x00100000

; Read kernel_num_sectors sectors starting from LBA 2 into 0x00100000
load_kernel:
    pushad
    
    mov edi, 0x00100000         ; Destination address
    mov eax, 2                  ; Starting LBA (third sector)
    mov ecx, kernel_num_sectors ; Number of sectors to read
    
.read_loop:
    push eax                    ; Save current LBA
    push ecx                    ; Save remaining sector count
    
    ; Wait for drive to be ready
    mov dx, 0x1F7               ; Status port

.wait_ready:
    in al, dx
    test al, 0x80               ; Test BSY bit
    jnz .wait_ready
    test al, 0x40               ; Test RDY bit
    jz .wait_ready
    
    pop ecx                     ; Restore sector count
    pop eax                     ; Restore current LBA
    push eax
    push ecx
    
    ; Select drive and set LBA mode (bits 24-27 of LBA)
    mov dx, 0x1F6               ; Drive/Head port
    mov ebx, eax
    shr ebx, 24
    and bl, 0x0F                ; Keep only lower 4 bits
    or bl, 0xE0                 ; Set LBA mode, master drive
    mov al, bl
    out dx, al
    
    ; Write sector count (1 sector at a time)
    mov dx, 0x1F2               ; Sector count port
    mov al, 1
    out dx, al
    
    pop ecx
    pop eax
    push eax
    push ecx
    
    ; Write LBA address (bits 0-7)
    mov dx, 0x1F3               ; LBA low byte
    out dx, al
    
    ; LBA bits 8-15
    mov dx, 0x1F4               ; LBA mid byte
    mov ebx, eax
    shr ebx, 8
    mov al, bl
    out dx, al
    
    pop ecx
    pop eax
    push eax
    push ecx
    
    ; LBA bits 16-23
    mov dx, 0x1F5               ; LBA high byte
    mov ebx, eax
    shr ebx, 16
    mov al, bl
    out dx, al
    
    ; Send READ SECTORS command
    mov dx, 0x1F7               ; Command port
    mov al, 0x20                ; READ SECTORS command
    out dx, al
    
    ; Wait for data to be ready

.wait_data:
    in al, dx
    test al, 0x08               ; Test DRQ (Data Request) bit
    jz .wait_data
    
    ; Read 256 words (512 bytes) from data port
    mov dx, 0x1F0               ; Data port
    mov ecx, 256                ; 256 words = 512 bytes
    rep insw                    ; Read words into [EDI]
    
    ; Move to next sector
    pop ecx                     ; Restore sector count
    pop eax                     ; Restore LBA
    inc eax                     ; Next LBA
    dec ecx                     ; One less sector to read
    jnz .read_loop              ; Continue if more sectors to read
    
    popad
    ret

kernel_num_sectors equ 100
code_segment equ gdt_kernel_code_segment - gdt_start
data_segment equ gdt_kernel_data_segment - gdt_start

times 512-($-$$) db 0