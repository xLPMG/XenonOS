#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "memory.h"

struct multiboot_info
{
    uint32_t flags;

    uint32_t mem_lower;
    uint32_t mem_upper;

    uint32_t boot_device;
    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    uint32_t syms[4];

    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed));

// One entry of the multiboot memory map.
// Next entry starts at (uint8_t *)entry + entry->size + sizeof(entry->size)
struct multiboot_mmap_entry
{
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

// Type 1 means the region is normal, available RAM
#define MMAP_TYPE_AVAILABLE 1

// Multiboot info flags
#define MULTIBOOT_FLAG_MEM (1 << 0)  // mem_lower / mem_upper valid
#define MULTIBOOT_FLAG_MMAP (1 << 6) // mmap_length / mmap_addr valid

#endif
