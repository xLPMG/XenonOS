#ifndef SLAB_H
#define SLAB_H

#include "memory.h"

typedef struct slab slab_t;

typedef struct slab_cache
{
    uint32_t object_size;
    slab_t *slabs;
} slab_cache_t;

void slab_cache_init(slab_cache_t *cache, uint32_t object_size);

void *slab_alloc(slab_cache_t *cache);
void slab_free(slab_cache_t *cache, void *object);

#endif
