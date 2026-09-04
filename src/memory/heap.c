#include "heap.h"
#include "slab.h"
#include "pmm.h"
#include "paging.h"
#include "constants.h"

// Binned free-list allocator
// different slab caches for different object sizes
static slab_cache_t kmalloc_cache_16;
static slab_cache_t kmalloc_cache_32;
static slab_cache_t kmalloc_cache_64;
static slab_cache_t kmalloc_cache_128;
static slab_cache_t kmalloc_cache_256;
static slab_cache_t kmalloc_cache_512;
static slab_cache_t kmalloc_cache_1024;
static slab_cache_t kmalloc_cache_2048;

// 'LRGE', deliberately not page-aligned so it can never be mistaken for a
// real (always page-aligned, or 0) slab_t::next value.
#define LARGE_ALLOC_MAGIC 0x4C524745u

typedef struct
{
    uint32_t magic;
    uint32_t page_count;
} large_header_t;

static uint32_t next_large_vaddr = LARGE_ALLOC_REGION_BASE;

void heap_initialize(void)
{
    slab_cache_init(&kmalloc_cache_16, 16);
    slab_cache_init(&kmalloc_cache_32, 32);
    slab_cache_init(&kmalloc_cache_64, 64);
    slab_cache_init(&kmalloc_cache_128, 128);
    slab_cache_init(&kmalloc_cache_256, 256);
    slab_cache_init(&kmalloc_cache_512, 512);
    slab_cache_init(&kmalloc_cache_1024, 1024);
    slab_cache_init(&kmalloc_cache_2048, 2048);
}

static void *kmalloc_large(uint32_t size)
{
    uint32_t total = size + sizeof(large_header_t);
    uint32_t pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

    if (next_large_vaddr + pages * PAGE_SIZE > LARGE_ALLOC_REGION_END)
        return 0; // region exhausted

    uint32_t vbase = next_large_vaddr;
    next_large_vaddr += pages * PAGE_SIZE;

    for (uint32_t i = 0; i < pages; i++)
    {
        uint32_t frame = pmm_alloc();
        if (frame == (uint32_t)-1)
            return 0;

        paging_map(vbase + i * PAGE_SIZE, frame, 3);
    }

    large_header_t *header = (large_header_t *)vbase;
    header->magic = LARGE_ALLOC_MAGIC;
    header->page_count = pages;

    return (void *)(header + 1);
}

static void kfree_large(large_header_t *header)
{
    uint32_t vbase = (uint32_t)header;
    uint32_t page_count = header->page_count;

    for (uint32_t i = 0; i < page_count; i++)
    {
        uint32_t vaddr = vbase + i * PAGE_SIZE;
        uint32_t frame = paging_get_physical(vaddr);
        paging_unmap(vaddr);
        pmm_free(frame);
    }

    // TODO: vbase itself is never reclaimed (bump allocator) same tradeoff as the thread stack region.
}

void *kmalloc(uint32_t size)
{
    if (size <= 16)
        return slab_alloc(&kmalloc_cache_16);
    else if (size <= 32)
        return slab_alloc(&kmalloc_cache_32);
    else if (size <= 64)
        return slab_alloc(&kmalloc_cache_64);
    else if (size <= 128)
        return slab_alloc(&kmalloc_cache_128);
    else if (size <= 256)
        return slab_alloc(&kmalloc_cache_256);
    else if (size <= 512)
        return slab_alloc(&kmalloc_cache_512);
    else if (size <= 1024)
        return slab_alloc(&kmalloc_cache_1024);
    else if (size <= 2048)
        return slab_alloc(&kmalloc_cache_2048);
    else
        return kmalloc_large(size);
}

void kfree(void *ptr)
{
    if (!ptr)
        return;

    void *page = (void *)((uint32_t)ptr & ~(PAGE_SIZE - 1));
    uint32_t tag = *(uint32_t *)page;

    if (tag == LARGE_ALLOC_MAGIC)
    {
        kfree_large((large_header_t *)page);
        return;
    }

    slab_t *slab = (slab_t *)page;
    slab_free(slab->cache, ptr);
}