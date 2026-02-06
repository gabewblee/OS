#ifndef TEST_FALLOC_H
#define TEST_FALLOC_H

#include <stdint.h>

#include "../../src/memory/falloc.h"

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
 * test_falloc_init - Test the falloc initialization
 *
 * Return: 0 on success, -1 on failure
 */
int test_falloc_init(void);

#endif