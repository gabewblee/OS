#ifndef FALLOC_H
#define FALLOC_H

#include <stdint.h>

#include "mmap.h"

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
 * ADDR_IO_START - Reserved I/O memory start
 */
#define ADDR_IO_START          0x00000000

/**
 * ADDR_IO_END - Reserved I/O memory end
 */
#define ADDR_IO_END            0x000FFFFF

/**
 * ADDR_KERNEL_START - Kernel memory start
 */
#define ADDR_KERNEL_START      0x00100000

/**
 * ADDR_KERNEL_END - Kernel memory end
 */
#define ADDR_KERNEL_END        0x004FFFFF

/**
 * ADDR_FREE_START - Free memory start
 */
#define ADDR_FREE_START        0x00500000

/**
 * ADDR_FREE_END - Free memory end
 */
#define ADDR_FREE_END          0xFFFFFFFF

/**
 * struct frame_allocator - Physical frame allocator state
 * @bitmap: Allocation bitmap (1 bit per 4 KiB page)
 */
typedef struct frame_allocator {
    uint32_t bitmap[BITMAP_SIZE];
} frame_allocator;

/**
 * get_frame_allocator - Get the frame allocator instance
 *
 * Return: Pointer to the frame allocator instance
 */
frame_allocator *get_frame_allocator(void);

/**
 * falloc_init - Initialize the frame allocator
 * @map: Pointer to the memory map
 *
 * Return: Nothing
 */
void falloc_init(const mmap_t *map);

/**
 * fallocate - Allocate a physical frame
 * @paddr: On success, set to the physical address of the frame
 *
 * Return: 0 on success, -1 on failure (no free frame)
 */
int fallocate(uint32_t *paddr);

/**
 * ffree - Free a previously allocated frame
 * @paddr: Physical address of the frame (as returned by fallocate())
 *
 * Return: Nothing
 */
void ffree(uint32_t paddr);

#endif