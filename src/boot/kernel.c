#include "../drivers/vga.h"
#include "../interrupts/idt.h"
#include "../memory/falloc.h"
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
}

/**
 * kernel_init - Initialize core kernel subsystems
 *
 * Return: Nothing
 */
void kernel_init(void) {
    /* Terminal */
    terminal_init();
    vga_print_string(0, 0, "Initialized terminal", WHITE, BLACK);

    /* Interrupts */
    // idt_init();
    // vga_print_string(1, 0, "Initialized interrupts", WHITE, BLACK);

    /* Memory */
    mmap_init(&mmap);
    vga_print_string(2, 0, "Initialized global memory map", WHITE, BLACK);

    falloc_init(&mmap);
    vga_print_string(3, 0, "Initialized page frame allocator", WHITE, BLACK);

    paging_init(&mmap);
    vga_print_string(4, 0, "Initialized paging", WHITE, BLACK);
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