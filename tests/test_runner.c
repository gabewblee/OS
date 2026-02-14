#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

#include "test_pmm.h"
#include "test_mmap.h"

/**
 * panic - Provide panic for code under test
 * @err: Error message
 *
 * Return: Nothing
 */
void panic(const char *err) {
    (void)err;
    fprintf(stderr, "panic (tests): %s\n", err ? err : "(null)");
    exit(1);
}

/**
 * run - Run all tests
 *
 * Return: 0 on success, 1 on failure
 */
static int run(void) {
    int failed = 0;

    if (test_pmm_init() != 0) {
        fprintf(stderr, "FAIL: test_pmm_init\n");
        failed = 1;
    } else {
        fprintf(stdout, "PASS: test_pmm_init\n");
    }

    if (test_mmap_init() != 0) {
        fprintf(stderr, "FAIL: test_mmap_init\n");
        failed = 1;
    } else {
        fprintf(stdout, "PASS: test_mmap_init\n");
    }

    return failed;
}

/**
 * main - Main function
 *
 * Return: 0 on success, 1 on failure
 */
int main(void) {
    return run() ? EXIT_FAILURE : EXIT_SUCCESS;
}

#endif
