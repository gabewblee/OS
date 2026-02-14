#include "vga.h"

/**
 * vga_row - Current cursor row (global)
 */
uint8_t vga_row = 0;

/**
 * vga_col - Current cursor column (global)
 */
uint8_t vga_col = 0;

/**
 * BLACK - Black vga color definition
 */
#define BLACK 0x00

/**
 * vga_enable_cursor - Enable the cursor
 * @cursor_start: Start position of the cursor
 * @cursor_end: End position of the cursor
 *
 * Return: Nothing
 */
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

/**
 * vga_disable_cursor - Disable the cursor
 *
 * Return: Nothing
 */
void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

/**
 * vga_update_cursor - Update the cursor position
 * @row: Row position
 * @col: Column position
 *
 * Return: Nothing
 */
void vga_update_cursor(uint8_t row, uint8_t col) {
    uint16_t pos = row * VGA_WIDTH + col;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

/**
 * vga_get_cursor_position - Get the cursor position
 *
 * Return: Cursor position
 */
uint16_t vga_get_cursor_position(void) {
    uint16_t pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}

/**
 * vga_set_cursor_position - Set the current cursor position
 * @row: Row position
 * @col: Column position
 *
 * Return: Nothing
 */
void vga_set_cursor_position(uint8_t row, uint8_t col) {
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH)
        return;

    vga_row = row;
    vga_col = col;
    vga_update_cursor(row, col);
}

/**
 * vga_newline - Move cursor to the start of the next line
 *
 * Return: Nothing
 */
void vga_newline(void) {
    vga_row++;
    vga_col = 0;
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll_up(BLACK);
        vga_row = VGA_HEIGHT - 1;
    }
}

/**
 * vga_scroll_up - Shift all lines up by one and clear the bottom line
 * @bcolor: Background color for the new empty line
 *
 * Return: Nothing
 */
void vga_scroll_up(unsigned char bcolor) {
    volatile uint16_t *buf = (uint16_t *)VGA_ADDR;
    uint16_t blank = (uint16_t)bcolor << 8 | ' ';
    const int penultimate_row = (VGA_HEIGHT - 1) * VGA_WIDTH;

    for (int i = 0; i < penultimate_row; i++)
        buf[i] = buf[i + VGA_WIDTH];

    for (int i = penultimate_row; i < penultimate_row + VGA_WIDTH; i++)
        buf[i] = blank;
}

/**
 * vga_print_char - Write a character at current cursor position and advance
 * @c: Character to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_char(char c, unsigned char fcolor, unsigned char bcolor) {
    if (c == '\n') {
        vga_newline();
        vga_update_cursor(vga_row, vga_col);
        return;
    }

    volatile uint16_t *buf = (uint16_t *)VGA_ADDR;
    uint16_t cell = (uint16_t)(unsigned char)c | ((uint16_t)((bcolor << 4) | (fcolor & 0x0F)) << 8);
    buf[vga_row * VGA_WIDTH + vga_col] = cell;
    vga_col++;
    if (vga_col >= VGA_WIDTH)
        vga_newline();

    vga_update_cursor(vga_row, vga_col);
}

/**
 * vga_print_string - Write a null-terminated string
 * @str: Null-terminated string to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_string(const char *str, unsigned char fcolor, unsigned char bcolor) {
    while (*str)
        vga_print_char(*str++, fcolor, bcolor);
}

/**
 * vga_clear_screen - Clear the screen with a single color
 * @color: Color to fill the screen with
 *
 * Return: Nothing
 */
void vga_clear_screen(unsigned char color) {
    volatile uint16_t *buf = (uint16_t *)VGA_ADDR;
    uint16_t blank = (uint16_t)color << 8 | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        buf[i] = blank;

    vga_row = 0;
    vga_col = 0;
    vga_update_cursor(0, 0);
}