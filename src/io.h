#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * outb - Write a byte to an I/O port
 * @port: I/O port to write to
 * @val: The byte value to write
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/**
 * inb - Read a byte from an I/O port
 * @port: I/O port to read from
 *
 * Return: The byte read from the port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

/**
 * outw - Write a word to an I/O port
 * @port: I/O port to write to
 * @val: The word value to write
 */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/**
 * inw - Read a word from an I/O port
 * @port: I/O port to read from
 *
 * Return: The word read from the port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %w1, %w0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

/**
 * io_wait - Wait for I/O operation to complete
 *
 * Writes to an unused port to create a small delay,
 * necessary between PIC commands.
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
