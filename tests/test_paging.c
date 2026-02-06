#define _GNU_SOURCE
#include <stddef.h>
#include <sys/mman.h>

#define mmap __os_mmap

#include "test_paging.h"
#include "../../src/memory/paging.h"
#include "../../src/memory/mmap.h"
#include "../../src/utils.h"

#undef mmap

/**
 * LOW_PAGE_HINT_0 - Low page hint for the first page table
 */
#define LOW_PAGE_HINT_0  ((void *)0x10000000)

/**
 * LOW_PAGE_HINT_1 - Low page hint for the second page table
 */
#define LOW_PAGE_HINT_1  ((void *)0x10001000)

/**
 * do_mmap - Map a page using mmap
 * @hint: Hint for the address to map
 * @out: Output parameter to store the mapped address
 *
 * Return: 0 on success, -1 on failure
 */
static int do_mmap(void *hint, void **out) {
    *out = mmap(hint, 
                PAGE_SIZE, 
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, 
                -1, 
                0
               );

    if (*out == MAP_FAILED)
        return -1;

    if ((uintptr_t)*out >> 12 > 0xFFFFFu)
        return -1;

    return 0;
}

/**
 * test_paging_init - Test get_paddr, map_page, unmap_page, first/last page of 4 GiB
 *
 * Return: 0 on success, -1 on failure
 */
int test_paging_init(void) {
    void *buf0 = NULL;
    void *buf1 = NULL;

    if (do_mmap(LOW_PAGE_HINT_0, &buf0) != 0)
        return -1;
    
    if (do_mmap(LOW_PAGE_HINT_1, &buf1) != 0) {
        munmap(buf0, PAGE_SIZE);
        return -1;
    }

    int ret = 0;
    uint32_t paddr = 0;

    /* First page of 4 GiB (0x00000000); same table will be used for 0x00100000 etc. */
    paging_test_setup(0x00000000, buf0);
    if (map_page(0x00100000, 0x00100000, PG_FLAG_RW) != 0) {
        ret = -1;
        goto out;
    }

    /* First page: start and end of block */
    if (get_paddr(0x00000000, &paddr) != 0 || paddr != 0x00000000) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00000FFF, &paddr) != 0 || paddr != 0x00000FFF) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00001000, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    /* Middle mapping (existing tests) */
    if (get_paddr(0x00100000, &paddr) != 0 || paddr != 0x00100000) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00100FFF, &paddr) != 0 || paddr != 0x00100FFF) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x000FFFFF, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00101000, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00500000, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    if (map_page(0x00200000, 0x00200000, PG_FLAG_RW) != 0) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00200000, &paddr) != 0 || paddr != 0x00200000) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00200FFF, &paddr) != 0 || paddr != 0x00200FFF) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00201000, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    if (unmap_page(0x00200000) != 0) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0x00200000, &paddr) != -1) {
        ret = -1;
        goto out;
    }

    if (map_page(0x00200001, 0x00200000, PG_FLAG_RW) != -1) {
        ret = -1;
        goto out;
    }

    if (map_page(0x00300000, 0x00300001, PG_FLAG_RW) != -1) {
        ret = -1;
        goto out;
    }

    if (unmap_page(0x00200001) != -1) {
        ret = -1;
        goto out;
    }

    /* Last page of 4 GiB (0xFFFFF000) */
    paging_test_setup_add(0xFFFFF000, buf1);
    if (get_paddr(0xFFFFF000, &paddr) != 0 || paddr != 0xFFFFF000) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0xFFFFFFFF, &paddr) != 0 || paddr != 0xFFFFFFFF) {
        ret = -1;
        goto out;
    }

    if (get_paddr(0xFFFFEFFF, &paddr) != -1) {
        ret = -1;
        goto out;
    }

out:
    munmap(buf0, PAGE_SIZE);
    if (buf1)
        munmap(buf1, PAGE_SIZE);

    return ret;
}
