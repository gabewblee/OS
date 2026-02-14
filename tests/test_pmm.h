#ifndef TEST_PMM_H
#define TEST_PMM_H

#include <stdint.h>

#include "../src/memory/pmm.h"

#define PAGE_SIZE               4096        /* System page size in bytes */
#define ADDR_IO_START           0x00000000  /* I/O memory start */
#define ADDR_FREE_END           0xFFFFFFFF  /* Free memory end */

/**
 * test_pmm_init - Test the pmm initialization
 *
 * Return: 0 on success, -1 on failure
 */
int test_pmm_init(void);

#endif
