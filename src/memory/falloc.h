#ifndef FALLOC_H
#define FALLOC_H

#include <stdint.h>

/**
 * WORD_SIZE - Bits per bitmap word
 */
#define WORD_SIZE              32

/**
 * PAGE_SIZE - System page size in bytes
 */
#define PAGE_SIZE              4096

/**
 * MAX_ADDR_SPACE_SIZE - Maximum physical address space size in bytes
 */
#define MAX_ADDR_SPACE_SIZE    0x100000000ULL

/**
 * MAX_NUM_PAGES - Maximum number of pages in address space
 */
#define MAX_NUM_PAGES          (MAX_ADDR_SPACE_SIZE / PAGE_SIZE)

/**
 * BITMAP_SIZE - Bitmap size in 32-bit words
 */
#define BITMAP_SIZE            (MAX_NUM_PAGES / WORD_SIZE)

/**
 * ADDR_RESERVED_START - Reserved low memory start (0 MiB)
 */
#define ADDR_RESERVED_START    0x00000000

/**
 * ADDR_RESERVED_END - Reserved low memory end (1 MiB)
 */
#define ADDR_RESERVED_END      0x00100000

/**
 * ADDR_KERNEL_START - Kernel physical start (1 MiB)
 */
#define ADDR_KERNEL_START      0x00100000

/**
 * struct frame_allocator - Physical frame allocator state
 * @bitmap: Allocation bitmap (1 bit per 4 KiB page)
 */
typedef struct frame_allocator {
    uint32_t bitmap[BITMAP_SIZE];
} frame_allocator;

/**
 * falloc_init - Initialize the frame allocator
 *
 * Return: Nothing
 */
void falloc_init(void);

/**
 * falloc_alloc - Allocate a physical frame
 *
 * Return: Physical address of the allocated frame, or 0 on failure
 */
uint32_t falloc_alloc(void);

/**
 * falloc_free - Free a previously allocated frame
 * @paddr: Physical address of the frame to free
 *
 * Return: Nothing
 */
void falloc_free(uint32_t paddr);

#endif