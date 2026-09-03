#include "slab.h"
#include "paging.h"
#include "pmm.h"
#include "constants.h"

// Slab header lives at the start of the page it describes
// the rest of the page is turned into object-sized chunks for allocation
struct slab
{
    struct slab *next;
    void *free_list;
    uint32_t free_count;
    uint32_t capacity;
};

void slab_cache_init(slab_cache_t *cache, uint32_t object_size)
{
    if (object_size < sizeof(void *))
        object_size = sizeof(void *);

    cache->object_size = object_size;
    cache->slabs = 0;
}

static slab_t *slab_create(slab_cache_t *cache)
{
    uint32_t frame = pmm_alloc();
    if (frame == (uint32_t)-1)
        return 0;

    // pmm_alloc frames aren't guaranteed to already be mapped (e.g. beyond
    // the first 4MB), so make sure the page is actually addressable.
    paging_map(frame, frame, 3);

    slab_t *slab = (slab_t *)frame;
    slab->next = 0;
    slab->free_list = 0;
    slab->free_count = 0;

    unsigned char *objects = (unsigned char *)frame + sizeof(slab_t);
    uint32_t usable = PAGE_SIZE - sizeof(slab_t);
    uint32_t count = usable / cache->object_size;

    for (uint32_t i = 0; i < count; i++)
    {
        void *object = objects + i * cache->object_size;
        *(void **)object = slab->free_list;
        slab->free_list = object;
        slab->free_count++;
    }

    slab->capacity = count;

    return slab;
}

static void *slab_take(slab_t *slab)
{
    void *object = slab->free_list;
    slab->free_list = *(void **)object;
    slab->free_count--;
    return object;
}

void *slab_alloc(slab_cache_t *cache)
{
    for (slab_t *slab = cache->slabs; slab; slab = slab->next)
    {
        if (slab->free_count > 0)
            return slab_take(slab);
    }

    slab_t *slab = slab_create(cache);
    if (!slab)
        return 0;

    slab->next = cache->slabs;
    cache->slabs = slab;

    return slab_take(slab);
}

void slab_free(slab_cache_t *cache, void *object)
{
    // Every slab is one page-aligned pmm frame, so masking the object's
    // address down to the page boundary recovers its owning slab header.
    slab_t *slab = (slab_t *)((uint32_t)object & ~(PAGE_SIZE - 1));

    *(void **)object = slab->free_list;
    slab->free_list = object;
    slab->free_count++;

    if (slab->free_count < slab->capacity)
        return;

    // Slab is now fully empty: unlink it and hand its page back to the pmm.
    slab_t **link = &cache->slabs; // // & of a "slab_t *" is a "slab_t **"
    while (*link != slab)
        link = &(*link)->next;
    *link = slab->next;

    pmm_free((uint32_t)slab);
}
