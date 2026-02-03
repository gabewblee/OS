#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/**
 * IDT_MAX_DESCRIPTORS - Number of entries in the IDT
 */
#define IDT_MAX_DESCRIPTORS 256

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
 * idt_init - Initialize the Interrupt Descriptor Table
 *
 * Return: Nothing
 */
void idt_init(void);

#endif
