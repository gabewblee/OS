#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#include "mmap.h"

#define PAGE_SIZE               4096                            /* System page size in bytes */
#define NUM_PAGE_ENTRIES        (PAGE_SIZE / sizeof(uint32_t))  /* Entries per directory/table */
#define ADDR_IDENTITY_START     0x00000000                      /* Start of identity mapping */
#define PG_FLAG_RW              0x01                            /* Read/write permission flag */
#define PG_FLAG_USER            0x02                            /* User/supervisor permission flag */

/**
 * struct pg_dir_entry_t - Page directory entry structure
 * @present: Present bit
 * @rw: Read/write bit
 * @user: User/supervisor bit
 * @pwt: Page-level write-through
 * @pcd: Page-level cache disable
 * @accessed: Accessed bit
 * @lower_avl: Lower 1 bits available for system programmer use
 * @ps: Page size (bit 0 = 4 KiB)
 * @upper_avl: Upper 4 bits available for system programmer use
 * @address: Physical address of the page table (bits 12-31)
 */
typedef struct pg_dir_entry_t {
    uint8_t present   : 1;
    uint8_t rw        : 1;
    uint8_t user      : 1;
    uint8_t pwt       : 1;
    uint8_t pcd       : 1;
    uint8_t accessed  : 1;
    uint8_t lower_avl : 1;
    uint8_t ps        : 1;
    uint8_t upper_avl : 4;
    uint32_t address  : 20;
} __attribute__((packed)) pg_dir_entry_t;

/**
 * struct pg_table_entry_t - Page table entry structure
 * @present: Present bit
 * @rw: Read/write bit
 * @user: User/supervisor bit
 * @pwt: Page-level write-through
 * @pcd: Page-level cache disable
 * @accessed: Accessed bit
 * @dirty: Dirty bit
 * @pat: Page attribute table
 * @global: Global page
 * @avl: 3 bits available for system programmer use
 * @address: Physical address of the page frame (bits 12-31)
 */
typedef struct pg_table_entry_t {
    uint8_t present  : 1;
    uint8_t rw       : 1;
    uint8_t user     : 1;
    uint8_t pwt      : 1;
    uint8_t pcd      : 1;
    uint8_t accessed : 1;
    uint8_t dirty    : 1;
    uint8_t pat      : 1;
    uint8_t global   : 1;
    uint8_t avl      : 3;
    uint32_t address : 20;
} __attribute__((packed)) pg_table_entry_t;

/**
 * get_paddr - Get the physical address of a virtual address
 * @vaddr: Virtual address
 * @paddr: On success, set to the physical address
 *
 * Return: 0 on success, -1 if not mapped
 */
int get_paddr(uint32_t vaddr, uint32_t *paddr);

/**
 * map - Map one virtual page to a physical frame
 * @vaddr: Virtual address (page-aligned)
 * @paddr: Physical address (page-aligned)
 * @flags: PG_FLAG_RW, PG_FLAG_USER, or 0
 *
 * Allocates a page table for the directory entry if needed. Invalidates TLB for @vaddr.
 * Return: 0 on success, -1 on failure
 */
int map(uint32_t vaddr, uint32_t paddr, uint32_t flags);

/**
 * unmap - Remove mapping for one virtual page
 * @vaddr: Virtual address (page-aligned)
 *
 * Clears the PTE. Invalidates TLB for @vaddr.
 * Return: 0 on success, -1 on failure
 */ 
int unmap(uint32_t vaddr);

/**
 * invalidate_tlb - Invalidate TLB entry for one virtual address
 * @vaddr: Virtual address
 *
 * Call after changing a mapping so the CPU uses the updated PTE.
 */
void invalidate_tlb(uint32_t vaddr);

/**
 * paging_init - Initialize paging
 *
 * Sets up paging structures and enables paging
 * @mmap: Pointer to the memory map
 *
 * Return: Nothing
 */
void paging_init(const mmap_t *mmap);

#endif