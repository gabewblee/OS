#include "idt.h"
#include "pic.h"
#include "../io.h"
#include "../drivers/vga.h"
#include "../utils.h"

/**
 * idt - Interrupt Descriptor Table
 */
__attribute__((aligned(0x10)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS];

/**
 * idtr - Interrupt Descriptor Table Register
 */
static idtr_t idtr;

extern void* isr_stub_table[];
extern void* irq_stub_table[];

static const char* exception_messages[] = {
    "#DE Division Error",
    "#DB Debug",
    "NMI Non-maskable Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR Bound Range Exceeded",
    "#UD Invalid Opcode",
    "#NM Device Not Available",
    "#DF Double Fault",
    "Coprocessor Segment Overrun",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack-Segment Fault",
    "#GP General Protection Fault",
    "#PF Page Fault",
    "Reserved",
    "#MF x87 Floating-Point Exception",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XM SIMD Floating-Point Exception",
    "#VE Virtualization Exception",
    "#CP Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "#HV Hypervisor Injection Exception",
    "#VC VMM Communication Exception",
    "#SX Security Exception",
    "Reserved"
};

__attribute__((noreturn))
void exception_handler(interrupt_frame_t* frame)
{
    vga_clear_screen(BLACK);
    
    if (frame->int_no < 32) {
        vga_print_string(0, 0, "EXCEPTION: ", RED, BLACK);
        vga_print_string(0, 11, exception_messages[frame->int_no], RED, BLACK);
    } else {
        vga_print_string(0, 0, "Unknown Exception", RED, BLACK);
    }
    
    __asm__ volatile ("cli; hlt");
    while (1);
}

volatile uint64_t timer_ticks = 0;
static uint8_t keyboard_row = 10;  /* Row to print scancodes */

void irq_handler(interrupt_frame_t* frame) {
    uint8_t irq = frame->int_no - 32;
    
    if (irq == 0) {
        timer_ticks++;
    } else if (irq == 1) {
        uint8_t scancode = inb(0x60);

        /* Print scancode as hex for now */
        char hex[] = "0x00";
        hex[2] = "0123456789ABCDEF"[scancode >> 4];
        hex[3] = "0123456789ABCDEF"[scancode & 0x0F];
        vga_print_string(keyboard_row, 0, hex, 0x0F, 0x00);
        keyboard_row++;
        if (keyboard_row > 24) keyboard_row = 10;
    }
    
    pic_send_eoi(irq);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low    = (uintptr_t)isr & 0xFFFF;
    descriptor->segment    = 0x08;
    descriptor->reserved   = 0;
    descriptor->attributes = flags;
    descriptor->isr_high   = (uintptr_t)isr >> 16;
}

void idt_init(void)
{
    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

    /* Exception handlers setup */
    for (uint8_t vector = 0; vector < 32; vector++)
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);

    /* IRQ handlers (32-33 for now) */
    idt_set_descriptor(32, irq_stub_table[0], 0x8E);  // IRQ0 - timer
    idt_set_descriptor(33, irq_stub_table[1], 0x8E);  // IRQ1 - keyboard

    __asm__ volatile ("lidt %0" : : "m"(idtr)); /* Load IDTR */
    /* Note: Caller should enable interrupts with sti after PIC is initialized */
}
