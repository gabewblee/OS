#include <stddef.h>

#include "pmm.h"
#include "../utils.h"

/* Frame allocator instance */
static pmm_t pmm;

/* Mark page-aligned [start,end] as used in the bitmap */
static void reserve(uint32_t start, uint32_t end) {
    uint64_t start_aligned = get_lower_alignment(start, PAGE_SIZE);
    uint64_t end_aligned = get_lower_alignment(end, PAGE_SIZE);
    for (uint64_t addr = start_aligned; addr <= end_aligned; addr += PAGE_SIZE) {
        uint32_t pg_number = (uint32_t)(addr / PAGE_SIZE);
        pmm.bitmap[pg_number / WORD_SIZE] |= (1U << (pg_number % WORD_SIZE));
    }
}

/* Return pointer to the frame allocator instance */
pmm_t *get_pmm(void) {
    return &pmm;
}

/* Clear bitmap and reserve all non-free sections from map */
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

/* Allocate one free frame; on success set *paddr and return 0, else -1 */
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

/* Mark the frame containing paddr as free in the bitmap */
void ffree(uint32_t paddr) {
    uint32_t page_number = paddr / PAGE_SIZE;
    uint32_t index = page_number / WORD_SIZE;
    uint32_t bit = page_number % WORD_SIZE;
    pmm.bitmap[index] &= ~(1U << bit);
}
