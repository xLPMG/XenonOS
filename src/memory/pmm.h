#ifndef PMM_H
#define PMM_H

#include "memory.h"
#include <constants.h>

void pmm_initialize(uint32_t multiboot_info_address);

uint32_t pmm_alloc(void);
void pmm_free(uint32_t address);

#endif