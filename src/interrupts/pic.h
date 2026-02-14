#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1		    0x20		/* IO base address for master PIC */
#define PIC2		    0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	    (PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	    (PIC2+1)

#define PIC_EOI         0x20        /* EOI Code */

#define ICW1_ICW4	    0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	    0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	    0x10		/* Initialization - required! */

#define ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	    0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	    0x10		/* Special fully nested (not) */

#define CASCADE_IRQ     2           /* IRQ number for cascading */

/* OCW3 commands for reading PIC registers */
#define PIC_READ_IRR    0x0a        /* Read IRQ Request Register */
#define PIC_READ_ISR    0x0b        /* Read In-Service Register */

/**
 * pic_init - Initialize the 8259 PIC
 * @offset1: Vector offset for master PIC (IRQ 0-7)
 * @offset2: Vector offset for slave PIC (IRQ 8-15)
 *
 * Initializes both PICs and remaps IRQs to the specified offsets.
 * Typically offset1=0x20 and offset2=0x28 to avoid conflict with
 * CPU exceptions (0x00-0x1F).
 */
void pic_init(uint8_t offset1, uint8_t offset2);

/**
 * pic_send_eoi - Send End-of-Interrupt signal to PIC
 * @irq: The IRQ number that was handled (0-15)
 *
 * Must be called at the end of every IRQ handler. For IRQs 8-15,
 * sends EOI to both slave and master PIC.
 */
void pic_send_eoi(uint8_t irq);

/**
 * pic_remap - Remap PIC IRQ vectors
 * @offset1: New vector offset for master PIC
 * @offset2: New vector offset for slave PIC
 *
 * Remaps the PIC interrupt vectors without full reinitialization.
 */
void pic_remap(int offset1, int offset2);

/**
 * pic_disable - Disable the 8259 PIC
 *
 * Masks all IRQs on both PICs. Should be called prior to switching to APIC
 */
void pic_disable(void);

/**
 * irq_set_mask - Mask a specific IRQ
 * @irq: The IRQ number to mask
 *
 * Prevents the specified IRQ from generating interrupts.
 */
void irq_set_mask(uint8_t irq);

/**
 * irq_clear_mask - Unmask a specific IRQ
 * @irq: The IRQ number to unmask
 *
 * Allows the specified IRQ to generate interrupts.
 */
void irq_clear_mask(uint8_t irq);

/**
 * pic_get_irr - Get combined value of IRQ request register
 *
 * Returns a 16-bit value indicating which IRQs are currently
 * requesting service (raised but not yet acknowledged).
 *
 * Return: Combined IRR from both PICs (bits 0-7 master, 8-15 slave)
 */
uint16_t pic_get_irr(void);

/**
 * pic_get_isr - Get combined value of IRQ in-service register
 *
 * Returns a 16-bit value indicating which IRQs are currently
 * being serviced (acknowledged but not yet EOI'd).
 *
 * Return: Combined ISR from both PICs (bits 0-7 master, 8-15 slave)
 */
uint16_t pic_get_isr(void);

#endif