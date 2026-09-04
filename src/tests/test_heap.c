#include "test.h"
#include "heap.h"
#include "string.h"
#include "constants.h"

static void test_small_allocation_is_usable(void)
{
    char *p = kmalloc(32);
    ASSERT(p != 0);

    memset(p, 0xAB, 32);
    for (int i = 0; i < 32; i++)
        ASSERT((unsigned char)p[i] == 0xAB);

    kfree(p);
}

static void test_distinct_allocations_dont_overlap(void)
{
    char *a = kmalloc(16);
    char *b = kmalloc(16);

    ASSERT(a != 0 && b != 0);
    ASSERT(a != b);

    memset(a, 0x11, 16);
    memset(b, 0x22, 16);

    for (int i = 0; i < 16; i++)
    {
        ASSERT((unsigned char)a[i] == 0x11);
        ASSERT((unsigned char)b[i] == 0x22);
    }

    kfree(a);
    kfree(b);
}

static void test_free_then_realloc_reuses_slab_slot(void)
{
    void *a = kmalloc(64);
    ASSERT(a != 0);
    kfree(a);

    // The slab this came from has exactly one free slot again, so the next
    // same-size request should land in the very same spot.
    void *b = kmalloc(64);
    ASSERT(b == a);

    kfree(b);
}

static void test_largest_bin_boundary(void)
{
    // 2048 is the biggest size still served by a slab bin.
    char *p = kmalloc(2048);
    ASSERT(p != 0);

    memset(p, 0x5A, 2048);
    ASSERT((unsigned char)p[0] == 0x5A);
    ASSERT((unsigned char)p[2047] == 0x5A);

    kfree(p);
}

static void test_large_allocation_spans_pages(void)
{
    // Bigger than the largest bin, so this must go through the large-alloc
    // path and span multiple mapped pages.
    uint32_t size = PAGE_SIZE * 3;
    char *p = kmalloc(size);
    ASSERT(p != 0);

    memset(p, 0x77, size);
    ASSERT((unsigned char)p[0] == 0x77);
    ASSERT((unsigned char)p[size - 1] == 0x77);

    kfree(p);
}

static void test_many_small_allocations_succeed(void)
{
    void *ptrs[64];

    for (int i = 0; i < 64; i++)
    {
        ptrs[i] = kmalloc(48);
        ASSERT(ptrs[i] != 0);
    }

    for (int i = 0; i < 64; i++)
        kfree(ptrs[i]);
}

static const test_case_t cases[] = {
    {"small_allocation_is_usable", test_small_allocation_is_usable},
    {"distinct_allocations_dont_overlap", test_distinct_allocations_dont_overlap},
    {"free_then_realloc_reuses_slab_slot", test_free_then_realloc_reuses_slab_slot},
    {"largest_bin_boundary", test_largest_bin_boundary},
    {"large_allocation_spans_pages", test_large_allocation_spans_pages},
    {"many_small_allocations_succeed", test_many_small_allocations_succeed},
};

void test_suite_heap(void)
{
    test_run_suite("heap", cases, sizeof(cases) / sizeof(cases[0]));
}
