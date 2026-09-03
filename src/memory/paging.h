#ifndef PAGING_H
#define PAGING_H

typedef unsigned int uint32_t;

void paging_initialize(void);

void paging_map(uint32_t virtual_address, uint32_t physical_address,
                uint32_t flags);

void paging_unmap(uint32_t virtual_address);

uint32_t paging_get_physical(uint32_t virtual_address);

#endif // PAGING_H