#ifndef FALLOC_H
#define FALLOC_H

#include <stdint.h>

#define WORD_SIZE           32
#define PAGE_SIZE           4096
#define MAX_ADDR_SPACE_SIZE 0x100000000ULL
#define MAX_NUM_PAGES       (MAX_ADDR_SPACE_SIZE / PAGE_SIZE)
#define BITMAP_SIZE         (MAX_NUM_PAGES / WORD_SIZE)

typedef struct frame_allocator {
    uint32_t bitmap[BITMAP_SIZE];
} frame_allocator;

frame_allocator falloc;

void falloc_init(void);
uint32_t falloc_alloc(void);
void falloc_free(uint32_t paddr);

#endif