#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

#include "test_pmm.h"
#include "test_mmap.h"

/* Test stub: print message and exit(1) */
void panic(const char *err) {
    (void)err;
    fprintf(stderr, "panic (tests): %s\n", err ? err : "(null)");
    exit(1);
}

/* Run all test suites; return 0 if all pass, 1 if any fail */
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

/* Entry point for test binary; returns EXIT_FAILURE if any test fails */
int main(void) {
    return run() ? EXIT_FAILURE : EXIT_SUCCESS;
}

#endif
