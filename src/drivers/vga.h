#ifndef VGA_H
#define VGA_H

#define VGA_ADDR    0xB8000 // VGA text mode memory-mapped I/O base address.
#define VGA_WIDTH   80      // VGA text mode screen width.
#define VGA_HEIGHT  25      // VGA text mode screen height.

/**
 * Writes a single character at the specified coordinates with foreground and background 
 * colors.
 * 
 * @param row The row position.
 * @param col The column position.
 * @param c The character to write.
 * @param fcolor The foreground color.
 * @param bcolor The background color.
 * @return void
 */
void vga_print_char(int row, int col, char c, unsigned char fcolor, unsigned char bcolor);

/**
 * Writes a null-terminated string starting at the  at the specified coordinates with foreground and 
 * background colors.
 * 
 * @param row The starting row position.
 * @param col The starting column position.
 * @param str The null-terminated string to write.
 * @param fcolor The foreground color.
 * @param bcolor The background color.
 * @return void
 */
void vga_print_string(int row, int col, const char* str, unsigned char fcolor, unsigned char bcolor);

/**
 * Clears the screen with the given color.
 * 
 * @param color The color to fill the screen with.
 * @return void
 */
void vga_clear_screen(unsigned char color);

#endif