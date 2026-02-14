#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#ifndef TEST
#include "drivers/vga.h"

#define BLACK  0x00    /* Black VGA color */
#define WHITE  0x0F    /* White VGA color */
#define RED    0x0C    /* Red VGA color */

/**
 * panic - Display a panic message and halt the system
 * @err: Error message to display
 *
 * Return: Nothing
 */
static inline void panic(const char *err) {
    vga_clear_screen(BLACK);
    vga_print_string(err, RED, BLACK);
    __asm__ volatile ("cli; hlt");
    while(1);
}
#else
/**
 * panic - Abort with message (host tests only; defined in test runner)
 */
void panic(const char *err);
#endif

/**
 * get_upper_alignment - Get the nearest upper page-aligned address
 * @addr: Input address
 * @pg_size: Page size
 *
 * Return: Aligned address
 */
static inline uint64_t get_upper_alignment(uint64_t addr, uint64_t pg_size) {
    return (addr + pg_size - 1) & ~(pg_size - 1);
}

/**
 * get_lower_alignment - Get the nearest lower page-aligned address
 * @addr: Input address
 * @pg_size: Page size
 *
 * Return: Aligned address
 */
static inline uint64_t get_lower_alignment(uint64_t addr, uint64_t pg_size) {
    return addr & ~(pg_size - 1);
}

#endif