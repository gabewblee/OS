#include "idt.h"

/**
 * idt - Interrupt Descriptor Table
 */
__attribute__((aligned(0x10)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS];

/**
 * idtr - Interrupt Descriptor Table Register
 */
static idtr_t idtr;

/**
 * isr_stub_table - Table of ISR stub addresses
 */
extern uint32_t isr_stub_table[];

/**
 * exception_handler - Default CPU exception handler
 *
 * Disables interrupts and halts the CPU
 *
 * Return: Does not return
 */
__attribute__((noreturn))
void exception_handler(void)
{
    __asm__ volatile ("cli; hlt");
    while (1);
}

/**
 * idt_set_descriptor - Set up an IDT entry
 * @vector: Interrupt number
 * @isr: Address of the ISR
 * @flags: Attributes (0x8E = present, ring 0, 32-bit interrupt gate)
 *
 * Splits the ISR address into low/high parts as required by x86
 *
 * Return: Nothing
 */
void idt_set_descriptor(uint8_t vector, uint32_t isr, uint8_t flags)
{
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low    = isr & 0xFFFF;
    descriptor->segment    = 0x08;
    descriptor->reserved   = 0;
    descriptor->attributes = flags;
    descriptor->isr_high   = isr >> 16;
}

/**
 * idt_init - Initialize the Interrupt Descriptor Table
 *
 * Sets up the IDTR and loads it into the CPU
 *
 * Return: Nothing
 */
void idt_init(void)
{
    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

    /* Exception handlers setup */
    for (uint8_t vector = 0; vector < 32; vector++)
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr)); /* Load IDTR */
    __asm__ volatile ("sti");   /* Enable interrupts */
}
