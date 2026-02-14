#ifndef TEST_PMM_H
#define TEST_PMM_H

#include <stdint.h>

#include "../src/memory/pmm.h"

/**
 * PAGE_SIZE - System page size in bytes
 */
#define PAGE_SIZE               4096

/**
 * ADDR_IO_START - I/O memory start
 */
 #define ADDR_IO_START          0x00000000
 
 /**
  * ADDR_FREE_END - Free memory end
  */
 #define ADDR_FREE_END          0xFFFFFFFF

/**
 * test_pmm_init - Test the pmm initialization
 *
 * Return: 0 on success, -1 on failure
 */
int test_pmm_init(void);

#endif
