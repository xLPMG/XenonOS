#ifndef SLAB_H
#define SLAB_H

#include "memory.h"

typedef struct slab slab_t;
typedef struct slab_cache slab_cache_t;

// Slab header lives at the start of the page it describes
// the rest of the page is turned into object-sized chunks for allocation
struct slab
{
    struct slab *next;   // pointer to the next slab in the cache's list
    slab_cache_t *cache; // pointer to the cache this slab belongs to
    void *free_list;     // pointer to the first free object in this slab
    uint32_t free_count; // how many objects are currently free in this slab
    uint32_t capacity;   // how many objects this slab can hold
};

struct slab_cache
{
    uint32_t object_size;
    slab_t *slabs;
};

void slab_cache_init(slab_cache_t *cache, uint32_t object_size);

void *slab_alloc(slab_cache_t *cache);
void slab_free(slab_cache_t *cache, void *object);

#endif
