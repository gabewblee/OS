#include "../drivers/ata_pio.h"
#include "../drivers/vga.h"
#include "../interrupts/idt.h"
#include "../interrupts/pic.h"
#include "../memory/pmm.h"
#include "../memory/paging.h"
#include "../memory/mmap.h"

/**
 * BLACK - Black vga color definition
 */
#define BLACK 0x00

/**
 * WHITE - White vga color definition
 */
#define WHITE 0x0F

/**
 * mmap - Global memory map instance
 */
mmap_t mmap;

/**
 * terminal_init - Initialize the VGA terminal
 *
 * Clears the screen and prints a startup banner
 *
 * Return: Nothing
 */
void terminal_init(void) {
    vga_clear_screen(BLACK);
    vga_enable_cursor(0, 15);
    vga_update_cursor(vga_row, vga_col);
}

/**
 * kernel_init - Initialize core kernel subsystems
 *
 * Return: Nothing
 */
void kernel_init(void) {
    terminal_init();
    vga_print_string("Initialized terminal\n", WHITE, BLACK);

    idt_init();
    vga_print_string("Initialized IDT\n", WHITE, BLACK);

    pic_init(0x20, 0x28);
    irq_clear_mask(0);                       /* Enable timer (IRQ0) */
    irq_clear_mask(1);                       /* Enable keyboard (IRQ1) */
    __asm__ volatile ("sti");                /* Enable interrupts */
    vga_print_string("Initialized PIC\n", WHITE, BLACK);

    mmap_init(&mmap);
    vga_print_string("Initialized global memory map\n", WHITE, BLACK);

    pmm_init(&mmap);
    vga_print_string("Initialized page frame allocator\n", WHITE, BLACK);

    ata_pio_init();
    vga_print_string("Initialized ATA PIO drivers\n", WHITE, BLACK);

    paging_init(&mmap);
    vga_print_string("Initialized paging\n", WHITE, BLACK);
}

/**
 * kmain - Kernel entry point
 *
 * Return: Does not return
 */
void kmain(void) {
    kernel_init();

    while(1) {
        ;
    }
}