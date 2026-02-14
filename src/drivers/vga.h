#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#include "../io.h"

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
 * vga_row - Current cursor row (global)
 */
extern uint8_t vga_row;

/**
 * vga_col - Current cursor column (global)
 */
extern uint8_t vga_col;

/**
 * vga_enable_cursor - Enable the cursor
 * @cursor_start: Start position of the cursor
 * @cursor_end: End position of the cursor
 *
 * Return: Nothing
 */
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

/**
 * vga_disable_cursor - Disable the cursor
 *
 * Return: Nothing
 */
void vga_disable_cursor(void);

/**
 * vga_update_cursor - Update the cursor position
 * @row: Row position
 * @col: Column position
 *
 * Return: Nothing
 */
void vga_update_cursor(uint8_t row, uint8_t col);

/**
 * vga_get_cursor_position - Get the cursor position
 *
 * Return: Cursor position
 */
uint16_t vga_get_cursor_position(void);

/**
 * vga_set_cursor_position - Set the current cursor position
 * @row: Row position
 * @col: Column position
 *
 * Return: Nothing
 */
void vga_set_cursor_position(uint8_t row, uint8_t col);

/**
 * vga_newline - Move cursor to the start of the next line
 *
 * Return: Nothing
 */
void vga_newline(void);

/**
 * vga_scroll_up - Shift all lines up by one and clear the bottom line
 * @bcolor: Background color for the new empty line
 *
 * Return: Nothing
 */
void vga_scroll_up(unsigned char bcolor);

/**
 * vga_print_char - Write a character at current cursor position and advance
 * @c: Character to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_char(char c, unsigned char fcolor, unsigned char bcolor);

/**
 * vga_print_string - Write a string and move to next line
 * @str: Null-terminated string to write
 * @fcolor: Foreground color
 * @bcolor: Background color
 *
 * Return: Nothing
 */
void vga_print_string(const char *str, unsigned char fcolor, unsigned char bcolor);

/**
 * vga_clear_screen - Clear the screen with a single color
 * @color: Color to fill the screen with
 *
 * Return: Nothing
 */
void vga_clear_screen(unsigned char color);

#endif