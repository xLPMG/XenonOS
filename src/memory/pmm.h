#ifndef PMM_H
#define PMM_H

#include "memory.h"

#define PAGE_SIZE 4096
#define MAX_MEMORY (4 * 1024 * 1024 * 1024ULL) // = 4 GB (is the max anyway)

// #frames = MAX_MEMORY / PAGE_SIZE
// each bit in the bitmap represents a frame (4 KB)
// 1 = used, 0 = free
// 32 bits per uint32_t
#define BITMAP_SIZE (MAX_MEMORY / PAGE_SIZE / 32)

void pmm_initialize(uint32_t multiboot_info_address);

uint32_t pmm_alloc(void);
void pmm_free(uint32_t address);

#endif