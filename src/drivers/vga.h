#ifndef VGA_H
#define VGA_H

/**
 * VGA_ADDR - VGA text mode memory-mapped I/O base address
 */
#define VGA_ADDR    0xB8000

/**
 * VGA_WIDTH - VGA text mode screen width
 */
#define VGA_WIDTH   80

/**
 * VGA_HEIGHT - VGA text mode screen height
 */
#define VGA_HEIGHT  25

/**
 * vga_print_char - Write a character at specified coordinates
 * @row: Row position
 * @col: Column position
 * @c: Character to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_char(int row, int col, char c, unsigned char fcolor, unsigned char bcolor);

/**
 * vga_print_string - Write a string starting at specified coordinates
 * @row: Starting row position
 * @col: Starting column position
 * @str: Null-terminated string to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_string(int row, int col, const char* str, unsigned char fcolor, unsigned char bcolor);

/**
 * vga_clear_screen - Clear the screen with a single color
 * @color: Color to fill the screen with
 *
 * Return: Nothing
 */
void vga_clear_screen(unsigned char color);

#endif