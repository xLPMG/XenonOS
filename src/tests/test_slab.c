#include "test.h"
#include "slab.h"

typedef struct
{
    int x;
    int y;
} point_t;

static void test_alloc_returns_distinct_objects(void)
{
    slab_cache_t cache;
    slab_cache_init(&cache, sizeof(point_t));

    point_t *a = slab_alloc(&cache);
    point_t *b = slab_alloc(&cache);

    ASSERT(a != 0);
    ASSERT(b != 0);
    ASSERT(a != b);

    a->x = 1;
    a->y = 2;
    b->x = 3;
    b->y = 4;

    ASSERT(a->x == 1 && a->y == 2);
    ASSERT(b->x == 3 && b->y == 4);

    slab_free(&cache, a);
    slab_free(&cache, b);
}

static void test_free_then_realloc_reuses_object(void)
{
    slab_cache_t cache;
    slab_cache_init(&cache, sizeof(point_t));

    point_t *a = slab_alloc(&cache);
    slab_free(&cache, a);

    point_t *b = slab_alloc(&cache);

    ASSERT(a == b);

    slab_free(&cache, b);
}

static void test_allocates_beyond_one_slab(void)
{
    slab_cache_t cache;
    slab_cache_init(&cache, sizeof(point_t));

    // A single 4KB page can't hold this many 8-byte objects, forcing a
    // second backing slab to be allocated.
    point_t *objects[700];

    for (int i = 0; i < 700; i++)
    {
        objects[i] = slab_alloc(&cache);
        ASSERT(objects[i] != 0);
        objects[i]->x = i;
    }

    ASSERT(cache.slabs != 0);

    for (int i = 0; i < 700; i++)
        ASSERT(objects[i]->x == i);

    for (int i = 0; i < 700; i++)
        slab_free(&cache, objects[i]);

    ASSERT(cache.slabs == 0);
}

static void test_reclaims_empty_slab(void)
{
    slab_cache_t cache;
    slab_cache_init(&cache, sizeof(point_t));

    point_t *object = slab_alloc(&cache);
    ASSERT(cache.slabs != 0);

    // Freeing the only object in this slab should make it fully empty,
    // causing it to be unlinked and its page handed back to the pmm.
    slab_free(&cache, object);
    ASSERT(cache.slabs == 0);
}

static const test_case_t cases[] = {
    {"alloc_returns_distinct_objects", test_alloc_returns_distinct_objects},
    {"free_then_realloc_reuses_object", test_free_then_realloc_reuses_object},
    {"allocates_beyond_one_slab", test_allocates_beyond_one_slab},
    {"reclaims_empty_slab", test_reclaims_empty_slab},
};

void test_suite_slab(void)
{
    test_run_suite("slab", cases, sizeof(cases) / sizeof(cases[0]));
}
