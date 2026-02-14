#include "idt.h"
#include "pic.h"
#include "../drivers/vga.h"
#include "../io.h"
#include "../utils.h"

#define NUM_EXCEPTIONS 32                        /* Number of CPU exception vectors */
#define NUM_IRQS       16                        /* Number of IRQ lines */

/* Interrupt Descriptor Table */
__attribute__((aligned(0x10)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS];

/* Interrupt Descriptor Table Register */
static idtr_t idtr;

/* ISR stub addresses */
extern void* isr_stub_table[];
/* IRQ stub addresses */
extern void* irq_stub_table[];

/* Exception messages */
static const char* exception_messages[] = {
    "#DE Division Error\n",
    "#DB Debug\n",
    "NMI Non-maskable Interrupt\n",
    "#BP Breakpoint\n",
    "#OF Overflow\n",
    "#BR Bound Range Exceeded\n",
    "#UD Invalid Opcode\n",
    "#NM Device Not Available\n",
    "#DF Double Fault\n",
    "Coprocessor Segment Overrun\n",
    "#TS Invalid TSS\n",
    "#NP Segment Not Present\n",
    "#SS Stack-Segment Fault\n",
    "#GP General Protection Fault\n",
    "#PF Page Fault\n",
    "Reserved\n",
    "#MF x87 Floating-Point Exception\n",
    "#AC Alignment Check\n",
    "#MC Machine Check\n",
    "#XM SIMD Floating-Point Exception\n",
    "#VE Virtualization Exception\n",
    "#CP Control Protection Exception\n",
    "Reserved\n", "Reserved\n", "Reserved\n", "Reserved\n", "Reserved\n", "Reserved\n",
    "#HV Hypervisor Injection Exception\n",
    "#VC VMM Communication Exception\n",
    "#SX Security Exception\n",
    "Reserved\n"
};

/* Display exception message and halt; called by ISR stubs for vectors 0–31 */
__attribute__((noreturn))
void exception_handler(interrupt_frame_t* frame)
{
    vga_clear_screen(BLACK);

    if (frame->int_no < NUM_EXCEPTIONS) {
        vga_print_string("EXCEPTION: ", RED, BLACK);
        vga_print_string(exception_messages[frame->int_no], RED, BLACK);
    } else {
        vga_print_string("Error: unknown exception\n", RED, BLACK);
    }
    
    __asm__ volatile ("cli; hlt");
    while (1);
}

/* Tick counter incremented by timer IRQ */
volatile uint64_t timer_ticks = 0;

/* Increment tick counter on timer IRQ */
void irq_pit_handler(void) {
    timer_ticks++;
}

/* Handle keyboard scancode from port 0x60 and print hex */
void irq_keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    /* Print scancode as hex for now */
    char hex[6] = "0x00";
    hex[2] = "0123456789ABCDEF"[scancode >> 4];
    hex[3] = "0123456789ABCDEF"[scancode & 0x0F];
    hex[4] = '\n';
    hex[5] = '\0';
    vga_print_string(hex, 0x0F, 0x00);
}

/* No-op for unused IRQ lines */
void irq_dummy_handler(void) {
    ;
}

/* Handle ATA primary bus IRQ (IRQ 14) */
void irq_ata_primary_handler(void) {
    ;
}

/* Handle ATA secondary bus IRQ (IRQ 15) */
void irq_ata_secondary_handler(void) {
    ;
}

/* Dispatch IRQ to device handler and send EOI; called by IRQ stubs for vectors 32+ */
void irq_handler(interrupt_frame_t* frame) {
    uint8_t irq = frame->int_no - NUM_EXCEPTIONS;
    switch(irq) {
        case 0:
            irq_pit_handler();
            break;
        case 1:
            irq_keyboard_handler();
            break;
        case 2:
            irq_dummy_handler();
            break;
        case 3:
            irq_dummy_handler();
            break;
        case 4:
            irq_dummy_handler();
            break;
        case 5:
            irq_dummy_handler();
            break;
        case 6:
            irq_dummy_handler();
            break;
        case 7:
            irq_dummy_handler();
            break;
        case 8:
            irq_dummy_handler();
            break;
        case 9:
            irq_dummy_handler();
            break;
        case 10:
            irq_dummy_handler();
            break;
        case 11:
            irq_dummy_handler();
            break;
        case 12:
            irq_dummy_handler();
            break;
        case 13:
            irq_dummy_handler();
            break;
        case 14:
            irq_ata_primary_handler();
            break;
        case 15:
            irq_ata_secondary_handler();
            break;
        default:
            panic("Error: unknown IRQ");
            break;
    }    
    pic_send_eoi(irq);
}

/* Set one IDT entry to point to the given ISR with the given attributes */
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
    idt_entry_t* descriptor = &idt[vector];
    descriptor->isr_low    = (uintptr_t)isr & 0xFFFF;
    descriptor->segment    = 0x08;
    descriptor->reserved   = 0;
    descriptor->attributes = flags;
    descriptor->isr_high   = (uintptr_t)isr >> 16;
}

/* Set up exception and IRQ handlers in IDT and load IDTR */
void idt_init(void)
{
    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

    /* Exception handlers setup */
    for (uint8_t vector = 0; vector < NUM_EXCEPTIONS; vector++)
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);

    /* IRQ handlers setup */
    for (uint8_t vector = 0; vector < NUM_IRQS; vector++)
        idt_set_descriptor(vector + NUM_EXCEPTIONS, irq_stub_table[vector], 0x8E);

    /* Load IDTR */
    __asm__ volatile ("lidt %0" : : "m"(idtr));
    /* Note: Caller should enable interrupts with sti after PIC is initialized */
}
