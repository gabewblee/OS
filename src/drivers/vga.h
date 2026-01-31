#ifndef VGA_H
#define VGA_H

#define VGA_ADDR    0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

void vga_print_char(int row, int col, char c, unsigned char fcolor, unsigned char bcolor);
void vga_print_string(int row, int col, const char* str, unsigned char fcolor, unsigned char bcolor);
void vga_clear_screen(unsigned char color);

#endif