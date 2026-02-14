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
static void register_section(mmap_t *map, uint32_t start, uint32_t end, mtype_t type) {
    if (map->count >= MAX_MEM_SECTIONS)
        panic("Error: failed to register more sections");

    if (start > end || start > ADDR_FREE_END || end > ADDR_FREE_END)
        panic("Error: section address out of range");

    if (type != SECTION_IO && type != SECTION_KERNEL && type != SECTION_FREE)
        panic("Error: section type invalid");
        
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
    if (!map)
        panic("Error: failed to initialize NULL memory map");

    map->count = 0;

    /* I/O memory */
    register_section(map, ADDR_IO_START, ADDR_IO_END, SECTION_IO);

    /* Kernel memory */
    register_section(map, ADDR_KERNEL_START, ADDR_KERNEL_END, SECTION_KERNEL);

    /* Free memory */
    register_section(map, ADDR_FREE_START, ADDR_FREE_END, SECTION_FREE);
}