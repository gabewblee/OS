#include "test_mmap.h"

#include "../../src/utils.h"

/**
 * test_mmap_init - Test the mmap initialization
 *
 * Return: 0 on success, -1 on failure
 */
int test_mmap_init(void) {
    mmap_t mmap;
    mmap_init(&mmap);

    if (mmap.count != 3)
        return -1;

    msection_t io_section = mmap.sections[0];
    if (io_section.start != ADDR_IO_START || io_section.end != ADDR_IO_END || io_section.type != SECTION_IO)
        return -1;
    
    msection_t kernel_section = mmap.sections[1];
    if (kernel_section.start != ADDR_KERNEL_START || kernel_section.end != ADDR_KERNEL_END || kernel_section.type != SECTION_KERNEL)
        return -1;

    msection_t free_section = mmap.sections[2];
    if (free_section.start != ADDR_FREE_START || free_section.end != ADDR_FREE_END || free_section.type != SECTION_FREE)
        return -1;

    return 0;
}

