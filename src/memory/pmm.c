#include <stddef.h>

#include "pmm.h"
#include "../utils.h"

/**
 * pmm - Frame allocator instance
 */
static pmm_t pmm;

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
        pmm.bitmap[pg_number / WORD_SIZE] |= (1U << (pg_number % WORD_SIZE));
    }
}

/**
 * get_pmm - Get the pmm instance
 *
 * Return: Pointer to the frame allocator instance
 */
pmm_t *get_pmm(void) {
    return &pmm;
}

/**
 * pmm_init - Initialize the pmm
 *
 * Reserves low memory, kernel space, and the allocator's bitmap storage
 * @map: Pointer to the memory map
 *
 * Return: Nothing
 */
void pmm_init(const mmap_t *map) {
    if (!map)
        panic("Error: failed to initialize NULL memory map");

    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        pmm.bitmap[i] = 0;

    for (uint32_t i = 0; i < map->count; i++) {
        const msection_t *section = &map->sections[i];
        if (section->type != SECTION_FREE)
            reserve(section->start, section->end);
    }
}

/**
 * falloc - Allocate a free physical frame
 * @paddr: On success, set to the physical address of the frame
 *
 * Return: 0 on success, -1 on failure
 */
int falloc(uint32_t *paddr) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        if (pmm.bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < WORD_SIZE; j++) {
                uint32_t mask = 1U << j;
                if (!(pmm.bitmap[i] & mask)) {
                    pmm.bitmap[i] |= mask;
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
    pmm.bitmap[index] &= ~(1U << bit);
}
