#include "mmap.h"

#include "../utils.h"

/**
 * register_section - Add a memory section to the memory map
 * @map: Pointer to the memory map
 * @start: Start address of the section
 * @end: End address of the section
 * @type: Type of the memory section
 *
 * Return: Nothing
 */
static void register_section(mmap_t *map, uint64_t start, uint64_t end, mtype_t type) {
    map->sections[map->count].start = start;
    map->sections[map->count].end = end;
    map->sections[map->count].type = type;
    map->count++;
}

/**
 * mmap_init - Initialize the memory map
 * @map: Pointer to the memory map to initialize
 *
 * Return: Nothing
 */
void mmap_init(mmap_t *map) {
    map->count = 0;

    /* I/O memory */
    register_section(map, ADDR_IO_START, ADDR_IO_END, SECTION_IO);

    /* Kernel memory */
    register_section(map, ADDR_KERNEL_START, ADDR_KERNEL_END, SECTION_KERNEL);

    /* Free memory */
    register_section(map, ADDR_FREE_START, ADDR_FREE_END, SECTION_FREE);
}