#include "paging.h"
#include "pmm.h"
#include "constants.h"

static uint32_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(4096)));
static uint32_t boot_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))); // kernel's initial page table

static inline void flush_tlb_entry(uint32_t virtual_address)
{
    // "r"(virtual_address) puts the address into a general-purpose register for the inline assembly to use
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");
}

// Virtual address of the page table backing a given directory index.
static inline uint32_t *page_table_virtual(uint32_t page_directory_index)
{
    return (uint32_t *)(RECURSIVE_TABLES_BASE + page_directory_index * PAGE_SIZE);
}

void paging_initialize(void)
{
    // clear all entries
    for (int i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
        page_directory[i] = 0x00000002; // bit1 = Writable, bit0 = not present

    page_directory[RECURSIVE_INDEX] = ((uint32_t)page_directory) | 3;

    // Identity-map the first 4MB so the kernel keeps running once paging turns on
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++)
        boot_page_table[i] = (i * PAGE_SIZE) | 3; // writable and present
    page_directory[0] = ((uint32_t)boot_page_table) | 3;

    // Load page directory into CR3
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));

    // Enable paging by setting the PG bit in CR0
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void paging_map(uint32_t virtual_address, uint32_t physical_address,
                uint32_t flags)
{
    // bits 11-0: offset in page
    // bits 21-12: table index
    // bits 31-22: directory index
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_table_index = (virtual_address >> 12) & 0x03FF;

    if (page_directory_index >= RECURSIVE_INDEX)
        return; // reserved for the recursive mapping window

    if (!(page_directory[page_directory_index] & 1))
    {
        uint32_t table_frame = pmm_alloc();
        if (table_frame == (uint32_t)-1)
            return;

        page_directory[page_directory_index] = (table_frame & 0xFFFFF000) | 3;
        flush_tlb_entry((uint32_t)page_table_virtual(page_directory_index));

        uint32_t *table = page_table_virtual(page_directory_index);
        for (int i = 0; i < PAGE_TABLE_ENTRIES; i++)
            table[i] = 0;
    }

    uint32_t *table = page_table_virtual(page_directory_index);
    table[page_table_index] = (physical_address & 0xFFFFF000) | (flags & 0xFFF);

    flush_tlb_entry(virtual_address);
}

void paging_unmap(uint32_t virtual_address)
{
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_table_index = (virtual_address >> 12) & 0x03FF;

    if (page_directory_index >= RECURSIVE_INDEX)
        return;

    if (!(page_directory[page_directory_index] & 1))
        return;

    uint32_t *table = page_table_virtual(page_directory_index);
    table[page_table_index] = 0;
    flush_tlb_entry(virtual_address);
}

uint32_t paging_get_physical(uint32_t virtual_address)
{
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_table_index = (virtual_address >> 12) & 0x03FF;

    if (page_directory_index >= RECURSIVE_INDEX)
        return 0;

    if (!(page_directory[page_directory_index] & 1))
        return 0;

    uint32_t *table = page_table_virtual(page_directory_index);
    return table[page_table_index] & 0xFFFFF000;
}