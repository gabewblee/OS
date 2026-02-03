#include "falloc.h"
#include "../drivers/vga.h"

void falloc_init(void) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        falloc.bitmap[i] = 0;
}

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

void falloc_free(uint32_t paddr) {
    uint32_t page_number = paddr / PAGE_SIZE;
    uint32_t index = page_number / WORD_SIZE;
    uint32_t bit = page_number % WORD_SIZE;
    falloc.bitmap[index] &= ~(1U << bit);
}