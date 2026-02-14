#include "pmm.h"
#include "paging.h"

#include "../utils.h"
#include "../drivers/vga.h"

/**
 * pg_dir - Page directory
 */
__attribute__((aligned(PAGE_SIZE)))
static pg_dir_entry_t pg_dir[NUM_PAGE_ENTRIES];

/**
 * pg_dir_entry_zero - Zero out a page directory entry
 * @entry: Page directory entry to zero
 *
 * Return: Nothing
 */
static void pg_dir_entry_zero(pg_dir_entry_t *pg_dir_entry) {
    pg_dir_entry->present   = 0;
    pg_dir_entry->rw        = 0;
    pg_dir_entry->user      = 0;
    pg_dir_entry->pwt       = 0;
    pg_dir_entry->pcd       = 0;
    pg_dir_entry->accessed  = 0;
    pg_dir_entry->lower_avl = 0;
    pg_dir_entry->ps        = 0;
    pg_dir_entry->upper_avl = 0;
    pg_dir_entry->address   = 0;
}

/**
 * pg_table_entry_zero - Zero out a page table entry
 * @entry: Page table entry to zero
 *
 * Return: Nothing
 */
static void pg_table_entry_zero(pg_table_entry_t *pg_table_entry) {
    pg_table_entry->present  = 0;
    pg_table_entry->rw       = 0;
    pg_table_entry->user     = 0;
    pg_table_entry->pwt      = 0;
    pg_table_entry->pcd      = 0;
    pg_table_entry->accessed = 0;
    pg_table_entry->dirty    = 0;
    pg_table_entry->pat      = 0;
    pg_table_entry->global   = 0;
    pg_table_entry->avl      = 0;
    pg_table_entry->address  = 0;
}

/**
 * mmap_find_kernel_section - Find the kernel memory section in the memory map
 * @mmap: Pointer to the memory map
 * @msection: Output parameter to store the found section
 *
 * Return: 0 on success, -1 if not found
 */
static int mmap_find_kernel_section(const mmap_t *mmap, msection_t *msection) {
    for (uint32_t i = 0; i < mmap->count; i++) {
        if (mmap->sections[i].type == SECTION_KERNEL) {
            *msection = mmap->sections[i];
            return 0;
        }
    }

    return -1;
}

/**
 * pg_dir_zero - Zero out a page directory
 * @pg_dir_entry: Page directory to zero
 *
 * Return: Nothing
 */
static void pg_dir_zero(pg_dir_entry_t *pg_dir_entry) {
    for (uint32_t i = 0; i < NUM_PAGE_ENTRIES; i++)
        pg_dir_entry_zero(&pg_dir_entry[i]);
}

/**
 * get_paddr - Get the physical address of a virtual address
 * @vaddr: Virtual address
 * @paddr: On success, set to the physical address
 *
 * Return: 0 on success, -1 if not mapped
 */
int get_paddr(uint32_t vaddr, uint32_t *paddr) {
    uint32_t pg_dir_index = (vaddr >> 22) & 0x3FF;
    uint32_t pg_table_index = (vaddr >> 12) & 0x3FF;

    pg_dir_entry_t *pg_dir_entry = &pg_dir[pg_dir_index];
    if (!pg_dir_entry->present)
        return -1;

    pg_table_entry_t *pg_table = (pg_table_entry_t *)(uintptr_t)(pg_dir_entry->address << 12);
    pg_table_entry_t *pg_table_entry = &pg_table[pg_table_index];
    if (!pg_table_entry->present)
        return -1;

    *paddr = (pg_table_entry->address << 12) | (vaddr & 0xFFF);
    return 0;
}

/**
 * map - Map one virtual page to a physical frame
 * @vaddr: Virtual address (page-aligned)
 * @paddr: Physical address (page-aligned)
 * @flags: PG_FLAG_RW, PG_FLAG_USER, or 0
 *
 * Allocates a page table for the directory entry if needed. Invalidates TLB for @vaddr.
 * Return: 0 on success, -1 on failure
 */
int map(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    if ((vaddr & 0xFFFFF000) != vaddr)
        return -1;

    if ((paddr & 0xFFFFF000) != paddr)
        return -1;

    uint32_t rw = (flags & PG_FLAG_RW) ? 1 : 0;
    uint32_t user = (flags & PG_FLAG_USER) ? 1 : 0;
    uint32_t pg_dir_index = (vaddr >> 22) & 0x3FF;
    pg_dir_entry_t *pg_dir_entry = &pg_dir[pg_dir_index];
    if (!pg_dir_entry->present) {
        uint32_t allocated;
        if (falloc(&allocated) == -1)
            return -1;
        
        pg_table_entry_t *new = (pg_table_entry_t *)(uintptr_t)allocated;
        for (uint32_t i = 0; i < NUM_PAGE_ENTRIES; i++)
            pg_table_entry_zero(&new[i]);
        
        pg_dir_entry_zero(pg_dir_entry);
        pg_dir_entry->present = 1;
        pg_dir_entry->rw = rw;
        pg_dir_entry->user = user;
        pg_dir_entry->address = allocated >> 12;
    }
    
    uint32_t pg_table_index = (vaddr >> 12) & 0x3FF;
    pg_table_entry_t *pg_table = (pg_table_entry_t *)(uintptr_t)(pg_dir_entry->address << 12);
    pg_table_entry_t *pg_table_entry = &pg_table[pg_table_index];
    if (pg_table_entry->present)
        return -1;

    /* User access is not allowed for supervisor pages */
    if (!pg_dir_entry->user && user)
        return -1;

    pg_table_entry_zero(pg_table_entry);
    pg_table_entry->present = 1;
    pg_table_entry->rw = rw;
    pg_table_entry->user = user;
    pg_table_entry->address = paddr >> 12;

    invalidate_tlb(vaddr);
    return 0;
}

/**
 * unmap - Remove mapping for one virtual page
 * @vaddr: Virtual address (page-aligned)
 *
 * Clears the PTE. Invalidates TLB for @vaddr.
 * Return: 0 on success, -1 on failure
 */ 
int unmap(uint32_t vaddr) {
    if ((vaddr & 0xFFFFF000) != vaddr)
        return -1;

    uint32_t pg_dir_index = (vaddr >> 22) & 0x3FF;
    pg_dir_entry_t *pg_dir_entry = &pg_dir[pg_dir_index];
    if (!pg_dir_entry->present)
        return -1;
    
    uint32_t pg_table_index = (vaddr >> 12) & 0x3FF;
    pg_table_entry_t *pg_table = (pg_table_entry_t *)(uintptr_t)(pg_dir_entry->address << 12);
    pg_table_entry_t *pg_table_entry = &pg_table[pg_table_index];
    if (!pg_table_entry->present)
        return -1;

    pg_table_entry_zero(pg_table_entry);
    invalidate_tlb(vaddr);
    return 0;
}

/**
 * invalidate_tlb - Invalidate TLB entry for one virtual address
 * @vaddr: Virtual address
 *
 * Uses invlpg (i486+). Call after changing a mapping so the CPU uses the updated PTE.
 */
void invalidate_tlb(uint32_t vaddr) {
    __asm__ volatile (
        "invlpg (%0)"
        :
        : "r"(vaddr)
        : "memory"
    );
}

/**
 * paging_kernel_space - Identity map the kernel space
 *
 * Sets up page tables to map the kernel's physical memory
 * @mmap: Pointer to the memory map
 *
 * Return: Nothing
 */
static void paging_kernel_space(const mmap_t *mmap) {
    msection_t kernel_section;
    if (mmap_find_kernel_section(mmap, &kernel_section) == -1)
        panic("Error: failed to find kernel section in memory map");

    uint64_t start_aligned = get_lower_alignment(ADDR_IO_START, PAGE_SIZE);
    uint64_t end_aligned = get_upper_alignment((uint64_t)kernel_section.end + 1, PAGE_SIZE);
    for (uint64_t addr = start_aligned; addr < end_aligned; ) {
        uint32_t pg_dir_index = (uint32_t)(addr >> 22) & 0x3FF;
        pg_dir_entry_t *pg_dir_entry = &pg_dir[pg_dir_index];
        pg_dir_entry_zero(pg_dir_entry);
        pg_dir_entry->present = 1;
        pg_dir_entry->rw = 1;

        uint32_t allocated;
        if (falloc(&allocated) == -1)
            panic("Error: failed to allocate frame");

        pg_table_entry_t *pg_table = (pg_table_entry_t *)(uintptr_t)allocated;
        for (uint32_t i = 0; i < NUM_PAGE_ENTRIES; i++)
            pg_table_entry_zero(&pg_table[i]);
        
        for (uint32_t i = 0; i < NUM_PAGE_ENTRIES && addr < end_aligned; i++) {
            pg_table_entry_t *pg_table_entry = &pg_table[i];
            pg_table_entry->present = 1;
            pg_table_entry->rw = 1;
            pg_table_entry->address = addr >> 12;
            addr += PAGE_SIZE;
        }

        pg_dir_entry->address = allocated >> 12;
    }
}

/**
 * paging_init - Initialize paging
 *
 * Sets up paging structures and enables paging
 * @mmap: Pointer to the memory map
 *
 * Return: Nothing
 */
void paging_init(const mmap_t *mmap) {
    if (!mmap)
        panic("Error: failed to initialize NULL memory map");

    /* Zero out the page directory */
    pg_dir_zero(pg_dir);

    /* Identity map the kernel space */
    paging_kernel_space(mmap);

    /* Load CR3 and enable paging */
    __asm__ volatile (
        "mov %%eax, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000001, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        : "a"(&pg_dir)
        : "memory"
    );
}
