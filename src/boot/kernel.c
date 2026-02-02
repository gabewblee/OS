#include "../drivers/vga.h"
#include "../interrupts/idt.h"

void init_terminal(void) {
    unsigned char black = 0x00;
    unsigned char white = 0x0F;
    vga_clear_screen(black);

    vga_print_string(0, 0, "Initializing Kernel...", white, black);
}

void init_kernel(void) {
    init_terminal();
    idt_init();
}

void kmain(void) {
    init_kernel();

    while(1) {
        ;
    }
}