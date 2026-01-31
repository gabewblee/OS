#include "vga.h"

void vga_print_char(int row, int col, char c, unsigned char fcolor, unsigned char bcolor) 
{
    volatile unsigned short* vga_buffer = (unsigned short*)VGA_ADDR;
    int offset = row * 80 + col;    

    unsigned char color = (bcolor << 4) | (fcolor & 0x0F);
    vga_buffer[offset] = (unsigned short)c | ((unsigned short)color << 8);
}

void vga_print_string(int row, int col, const char* str, unsigned char fcolor, unsigned char bcolor) {
    while (*str) {
        vga_print_char(row, col, *str++, fcolor, bcolor);
        col++;
        if (col >= VGA_WIDTH) {
            col = 0;
            row++;
        }
    }
}

void vga_clear_screen(unsigned char color) {
    for (int row = 0; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_print_char(row, col, ' ', color, color);
        }
    }
}