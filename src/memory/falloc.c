#include <stddef.h>

#include "falloc.h"

#include "../utils.h"

/**
 * falloc - Frame allocator instance
 */
static frame_allocator falloc;

/**
 * reserve - Mark a physical address range as used
 * @start: Start physical address (inclusive)
 * @end: End physical address (inclusive)
 *
 * Aligns the range to page boundaries and sets corresponding bitmap bits
 *
 * Return: Nothing
 */
static void reserve(uint32_t start, uint32_t end) {
    uint64_t start_aligned = get_lower_alignment(start, PAGE_SIZE);
    uint64_t end_aligned = get_lower_alignment(end, PAGE_SIZE);
    for (uint64_t addr = start_aligned; addr <= end_aligned; addr += PAGE_SIZE) {
        uint32_t pg_number = (uint32_t)(addr / PAGE_SIZE);
        falloc.bitmap[pg_number / WORD_SIZE] |= (1U << (pg_number % WORD_SIZE));
    }
}

/**
 * get_frame_allocator - Get the frame allocator instance
 *
 * Return: Pointer to the frame allocator instance
 */
frame_allocator *get_frame_allocator(void) {
    return &falloc;
}

/**
 * falloc_init - Initialize the frame allocator
 *
 * Reserves low memory, kernel space, and the allocator's bitmap storage
 * @map: Pointer to the memory map
 *
 * Return: Nothing
 */
void falloc_init(const mmap_t *map) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        falloc.bitmap[i] = 0;

    if (!map)
        panic("Error: NULL memory map passed to falloc_init");

    for (uint32_t i = 0; i < map->count; i++) {
        const msection_t *section = &map->sections[i];
        if (section->type != SECTION_FREE)
            reserve(section->start, section->end);
    }
}

/**
 * fallocate - Allocate a free physical frame
 * @paddr: On success, set to the physical address of the frame
 *
 * Return: 0 on success, -1 on failure
 */
int fallocate(uint32_t *paddr) {
    if (!paddr)
        return -1;

    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        if (falloc.bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < WORD_SIZE; j++) {
                uint32_t mask = 1U << j;
                if (!(falloc.bitmap[i] & mask)) {
                    falloc.bitmap[i] |= mask;
                    *paddr = (i * WORD_SIZE + j) * PAGE_SIZE;
                    return 0;
                }
            }
        }
    }
    return -1;
}

/**
 * ffree - Free a previously allocated frame
 * @paddr: Physical address of the frame
 *
 * Return: Nothing
 */
void ffree(uint32_t paddr) {
    uint32_t page_number = paddr / PAGE_SIZE;
    uint32_t index = page_number / WORD_SIZE;
    uint32_t bit = page_number % WORD_SIZE;
    falloc.bitmap[index] &= ~(1U << bit);
}