#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_MAX_DESCRIPTORS  256    /* Number of entries in the IDT */

/**
 * struct idt_entry_t - 32-bit IDT entry
 * @isr_low: Lower 16 bits of ISR address
 * @segment: GDT segment selector for CS
 * @reserved: Always 0
 * @attributes: Type and attributes
 * @isr_high: Upper 16 bits of ISR address
 */
typedef struct {
    uint16_t isr_low;
    uint16_t segment;
    uint8_t  reserved;
    uint8_t  attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry_t;

/**
 * struct idtr_t - Pointer structure for lidt instruction
 * @limit: Byte size of IDT - 1
 * @base: Pointer to idt[0] (where IDT starts)
 */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

/**
 * struct interrupt_frame_t - Stack frame passed to interrupt handlers
 * @edi, esi, ebp, esp, ebx, edx, ecx, eax: Registers saved by pushad
 * @int_no: Interrupt vector number (pushed by ISR stub)
 * @err_code: Error code (pushed by CPU or dummy 0 by stub)
 * @eip: Instruction pointer (pushed by CPU)
 * @cs: Code segment (pushed by CPU)
 * @eflags: Flags register (pushed by CPU)
 *
 * This structure represents the stack layout when an interrupt handler
 * is called. Used to access interrupt number and saved register state.
 *
 * Note: For exceptions with error codes, CPU pushes err_code first,
 * then stub pushes int_no. For exceptions without error codes,
 * stub pushes dummy 0 then int_no. Stack order: int_no is at lower address.
 */
typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip, cs, eflags;
} interrupt_frame_t;

/**
 * timer_ticks - Global tick counter incremented by timer IRQ
 */
extern volatile uint64_t timer_ticks;

/**
 * idt_init - Initialize the Interrupt Descriptor Table
 *
 * Sets up exception handlers (vectors 0-31) and IRQ handlers
 * (vectors 32+), then loads the IDT into the CPU.
 */
void idt_init(void);

/**
 * idt_set_descriptor - Set up an IDT entry
 * @vector: Interrupt vector number (0-255)
 * @isr: Address of the interrupt service routine
 * @flags: Attributes (0x8E = present, ring 0, 32-bit interrupt gate)
 */
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

/**
 * exception_handler - CPU exception handler
 * @frame: Pointer to interrupt stack frame
 *
 * Called by ISR stubs for vectors 0-31. Displays error and halts.
 */
void exception_handler(interrupt_frame_t* frame);

/**
 * irq_handler - Hardware interrupt handler
 * @frame: Pointer to interrupt stack frame
 *
 * Called by IRQ stubs for vectors 32+. Dispatches to appropriate
 * device handler and sends EOI to PIC.
 */
void irq_handler(interrupt_frame_t* frame);

#endif
