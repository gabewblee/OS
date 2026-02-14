#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#include "mmap.h"

#define WORD_SIZE              32                                   /* Bits per bitmap word */
#define PAGE_SIZE              4096                                 /* System page size in bytes */
#define MAX_ADDR_SPACE_SIZE    0x100000000ULL                       /* Max physical address space in bytes */
#define MAX_NUM_PAGES          (MAX_ADDR_SPACE_SIZE / PAGE_SIZE)    /* Max number of pages */
#define BITMAP_SIZE            (MAX_NUM_PAGES / WORD_SIZE)          /* Bitmap size in 32-bit words */
#define ADDR_IO_START          0x00000000                           /* Reserved I/O memory start */
#define ADDR_IO_END            0x000FFFFF                           /* Reserved I/O memory end */
#define ADDR_KERNEL_START      0x00100000                           /* Kernel memory start */
#define ADDR_KERNEL_END        0x004FFFFF                           /* Kernel memory end */
#define ADDR_FREE_START        0x00500000                           /* Free memory start */
#define ADDR_FREE_END          0xFFFFFFFF                           /* Free memory end */

/**
 * struct pmm_t - Physical memory manager
 * @bitmap: Allocation bitmap (1 bit per 4 KiB page)
 */
typedef struct pmm_t {
    uint32_t bitmap[BITMAP_SIZE];
} pmm_t;

/**
 * get_pmm - Get the pmm instance
 *
 * Return: Pointer to the pmm instance
 */
pmm_t *get_pmm(void);

/**
 * pmm_init - Initialize the pmm
 * @map: Pointer to the memory map
 *
 * Return: Nothing
 */
void pmm_init(const mmap_t *map);

/**
 * falloc - Allocate a physical frame
 * @paddr: On success, set to the physical address of the frame
 *
 * Return: 0 on success, -1 on failure (no free frame)
 */
int falloc(uint32_t *paddr);

/**
 * ffree - Free a previously allocated frame
 * @paddr: Physical address of the frame (as returned by falloc())
 *
 * Return: Nothing
 */
void ffree(uint32_t paddr);

#endif
