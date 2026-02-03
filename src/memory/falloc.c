#include "falloc.h"

static frame_allocator falloc;

/**
 * reserve - Mark a physical address range as used
 * @start: Start physical address (inclusive)
 * @end: End physical address (exclusive)
 *
 * Aligns the range to page boundaries and sets corresponding bitmap bits
 *
 * Return: Nothing
 */
static void reserve(uint32_t start, uint32_t end) {
    start &= ~(PAGE_SIZE - 1);
    end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        uint32_t page = addr / PAGE_SIZE;
        falloc.bitmap[page / WORD_SIZE] |= (1U << (page % WORD_SIZE));
    }
}

/**
 * falloc_init - Initialize the frame allocator
 *
 * Reserves low memory, kernel space, and the allocator's bitmap storage
 *
 * Return: Nothing
 */
void falloc_init(void) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        falloc.bitmap[i] = 0;

    // Reserve low memory
    reserve(ADDR_RESERVED_START, ADDR_RESERVED_END);

    // Reserve kernel space
    extern uint32_t kend;
    reserve(ADDR_KERNEL_START, (uint32_t)&kend);

    uint32_t bitmap_start = (uint32_t)&falloc.bitmap;
    uint32_t bitmap_end   = bitmap_start + sizeof(falloc.bitmap);
    reserve(bitmap_start, bitmap_end);
}

/**
 * falloc_alloc - Allocate a free physical frame
 *
 * Return: Physical address of the allocated frame, or 0 on failure
 */
uint32_t falloc_alloc(void) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        if (falloc.bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < WORD_SIZE; j++) {
                uint32_t mask = 1U << j;
                if (!(falloc.bitmap[i] & mask)) {
                    falloc.bitmap[i] |= mask;
                    return (i * WORD_SIZE + j) * PAGE_SIZE;
                }
            }
        }
    }
    return 0;
}

/**
 * falloc_free - Free a previously allocated frame
 * @paddr: Physical address of the frame to free
 *
 * Return: Nothing
 */
void falloc_free(uint32_t paddr) {
    uint32_t page_number = paddr / PAGE_SIZE;
    uint32_t index = page_number / WORD_SIZE;
    uint32_t bit = page_number % WORD_SIZE;
    falloc.bitmap[index] &= ~(1U << bit);
}