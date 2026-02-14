#ifndef MMAP_H
#define MMAP_H

#include <stdint.h>

#define MAX_MEM_SECTIONS        16          /* Maximum number of memory sections */
#define ADDR_IO_START           0x00000000  /* I/O memory start */
#define ADDR_IO_END             0x000FFFFF  /* I/O memory end */
#define ADDR_KERNEL_START       0x00100000  /* Kernel memory start */
#define ADDR_KERNEL_END         0x004FFFFF  /* Kernel memory end */
#define ADDR_FREE_START         0x00500000  /* Free memory start */
#define ADDR_FREE_END           0xFFFFFFFF  /* Free memory end */

/**
 * enum mem_type_t - Memory section types
 * @SECTION_IO: I/O memory
 * @SECTION_KERNEL: Kernel memory
 * @SECTION_FREE: Free memory
 */
typedef enum {
    SECTION_IO,
    SECTION_KERNEL,
    SECTION_FREE
} mtype_t;

/**
 * struct msection_t - Memory section structure
 * @start: Start address of the section
 * @end: End address of the section
 * @type: Type of the memory section
 */
typedef struct {
    uint32_t start;
    uint32_t end;
    mtype_t type;
} msection_t;

/**
 * struct mmap_t - Memory map structure
 * @sections: Array of memory sections
 * @count: Number of valid sections
 */
typedef struct {
    msection_t sections[MAX_MEM_SECTIONS];
    uint32_t count;
} mmap_t;

/**
 * mmap - Global memory map instance
 */
extern mmap_t mmap;

/**
 * mmap_init - Initialize the memory map
 * @map: Pointer to the memory map to initialize
 *
 * Return: Nothing
 */
void mmap_init(mmap_t *map);

#endif