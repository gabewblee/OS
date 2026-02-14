#include "test_pmm.h"

#include "../src/utils.h"

/**
 * count_num_bits_set - Count set bits in the bitmap (reserved pages)
 * @x: Bitmap word
 *
 * Return: Number of set bits
 */
static uint32_t count_num_bits_set(uint32_t x) {
    uint32_t n = 0;
    while (x) {
        n += x & 1u;
        x >>= 1;
    }
    return n;
}

/**
 * test_pmm_init - Test the pmm initialization
 *
 * Return: 0 on success, -1 on failure
 */
int test_pmm_init(void) {
    mmap_t mmap;
    mmap_init(&mmap);

    pmm_init(&mmap);
    pmm_t *pmm = get_pmm();
    uint32_t num_allocated = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        num_allocated += count_num_bits_set(pmm->bitmap[i]);

    uint32_t expected = (ADDR_KERNEL_END - ADDR_IO_START + 1) / PAGE_SIZE;
    return num_allocated != expected ? -1 : 0;
}
