#include "test.h"
#include "pmm.h"

static void test_alloc_returns_valid_frame(void)
{
    uint32_t frame = pmm_alloc();

    ASSERT(frame != (uint32_t)-1);

    pmm_free(frame);
}

static void test_alloc_returns_distinct_frames(void)
{
    uint32_t a = pmm_alloc();
    uint32_t b = pmm_alloc();

    ASSERT(a != b);

    pmm_free(a);
    pmm_free(b);
}

static void test_free_then_realloc_reuses_frame(void)
{
    uint32_t a = pmm_alloc();
    pmm_free(a);

    uint32_t b = pmm_alloc();

    ASSERT(a == b);

    pmm_free(b);
}

static const test_case_t cases[] = {
    {"alloc_returns_valid_frame", test_alloc_returns_valid_frame},
    {"alloc_returns_distinct_frames", test_alloc_returns_distinct_frames},
    {"free_then_realloc_reuses_frame", test_free_then_realloc_reuses_frame},
};

void test_suite_pmm(void)
{
    test_run_suite("pmm", cases, sizeof(cases) / sizeof(cases[0]));
}
