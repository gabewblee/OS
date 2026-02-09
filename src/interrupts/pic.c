#include "pic.h"
#include "../io.h"

void pic_init(uint8_t offset1, uint8_t offset2) {
    pic_remap(offset1, offset2);
}

void pic_remap(int offset1, int offset2) {
    /* ICW1: Start initialization sequence (cascade mode) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Set vector offsets */
    outb(PIC1_DATA, offset1);       /* Master PIC vector offset */
    io_wait();
    outb(PIC2_DATA, offset2);       /* Slave PIC vector offset */
    io_wait();

    /* ICW3: Configure cascading */
    outb(PIC1_DATA, 1 << CASCADE_IRQ);  /* Tell master about slave at IRQ2 */
    io_wait();
    outb(PIC2_DATA, CASCADE_IRQ);       /* Tell slave its cascade identity */
    io_wait();

    /* ICW4: Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Unmask both PICs */
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_disable(void) {
    /* Mask all interrupts */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void irq_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t mask;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    mask = inb(port) | (1 << irq);
    outb(port, mask);
}

void irq_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t mask;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    mask = inb(port) & ~(1 << irq);
    outb(port, mask);
}

/**
 * __pic_get_irq_reg - Read a PIC register via OCW3
 * @ocw3: The OCW3 command (PIC_READ_IRR or PIC_READ_ISR)
 *
 * Return: Combined 16-bit value from both PICs
 */
static uint16_t __pic_get_irq_reg(int ocw3) {
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_get_irr(void) {
    return __pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(void) {
    return __pic_get_irq_reg(PIC_READ_ISR);
}
