#include "../drivers/vga.h"
#include "../interrupts/idt.h"
#include "../memory/falloc.h"

/**
 * terminal_init - Initialize the VGA terminal
 *
 * Clears the screen and prints a startup banner
 *
 * Return: Nothing
 */
void terminal_init(void) {
    unsigned char black = 0x00;
    unsigned char white = 0x0F;
    vga_clear_screen(black);

    vga_print_string(0, 0, "Initializing Kernel...", white, black);
}

/**
 * kernel_init - Initialize core kernel subsystems
 *
 * Return: Nothing
 */
void kernel_init(void) {
    terminal_init();
    idt_init();
    falloc_init();
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