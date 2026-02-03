#ifndef FALLOC_H
#define FALLOC_H

#include <stdint.h>

// Constants
#define WORD_SIZE           32
#define PAGE_SIZE           4096
#define MAX_ADDR_SPACE_SIZE 0x100000000ULL
#define MAX_NUM_PAGES       (MAX_ADDR_SPACE_SIZE / PAGE_SIZE)
#define BITMAP_SIZE         (MAX_NUM_PAGES / WORD_SIZE)

// Kernel memory regions
#define ADDR_RESERVED_START    0x00000000 // 0 MiB
#define ADDR_RESERVED_END      0x00100000 // 1 MiB
#define ADDR_KERNEL_START      0x00100000 // 1 MiB

typedef struct frame_allocator {
    uint32_t bitmap[BITMAP_SIZE];
} frame_allocator;

/**
 * Initializes the frame allocator.
 * 
 * @return void
 */
void falloc_init(void);

/**
 * Allocates a frame and returns its physical address.
 * 
 * @return The physical address of the allocated frame, or 0 if allocation fails.
 */
uint32_t falloc_alloc(void);

/**
 * Frees the frame at the given physical address.
 * 
 * @param paddr The physical address of the frame to free.
 * @return void
 */
void falloc_free(uint32_t paddr);

#endif