#include "../drivers/vga.h"
#include "../interrupts/idt.h"
#include "../memory/falloc.h"

void terminal_init(void) {
    unsigned char black = 0x00;
    unsigned char white = 0x0F;
    vga_clear_screen(black);

    vga_print_string(0, 0, "Initializing Kernel...", white, black);
}

void kernel_init(void) {
    terminal_init();
    idt_init();
    falloc_init();
}

void kmain(void) {
    kernel_init();

    while(1) {
        ;
    }
}