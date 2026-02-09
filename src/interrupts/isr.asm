bits 32

extern exception_handler
extern irq_handler

; Macro for ISRs that don't push an error code
; We push: dummy err_code, then int_no
; Stack after: [int_no] [err_code=0] <- esp points here
%macro isr_no_err_stub 1
isr_stub_%+%1:
    push dword 0          ; dummy error code
    push dword %1         ; interrupt number
    jmp isr_handler_common
%endmacro

; Macro for ISRs that push an error code
; CPU pushes: err_code
; We push: int_no
; Stack after: [int_no] [err_code] <- esp points here
%macro isr_err_stub 1
isr_stub_%+%1:
    push dword %1         ; interrupt number (error code already on stack)
    jmp isr_handler_common
%endmacro

; Macro for IRQ handlers
%macro irq_stub 2
irq_stub_%+%1:
    push dword 0          ; dummy error code
    push dword %2         ; interrupt vector number
    jmp irq_handler_common
%endmacro

; Common handler for exceptions
isr_handler_common:
    pushad                ; save all registers
    cld
    push esp              ; pass pointer to stack frame as argument
    call exception_handler
    add esp, 4            ; clean up argument
    popad
    add esp, 8            ; clean up error code and interrupt number
    iret

; Common handler for IRQs
irq_handler_common:
    pushad
    cld
    push esp              ; pass pointer to stack frame as argument
    call irq_handler
    add esp, 4            ; clean up argument
    popad
    add esp, 8
    iret

; Define exception handlers
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

; IRQ stubs (mapped to vectors 0x20-0x2F)
irq_stub 0, 32    ; Timer -> vector 0x20
irq_stub 1, 33    ; Keyboard -> vector 0x21

; Table of ISR stub addresses
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 32
    dd isr_stub_%+i
%assign i i+1
%endrep

global irq_stub_table
irq_stub_table:
    dd irq_stub_0
    dd irq_stub_1
