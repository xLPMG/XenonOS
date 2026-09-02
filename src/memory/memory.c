#include "memory.h"

struct multiboot_info {
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

/*
 * One entry of the multiboot memory map.
 * Next entry starts at (uint8_t *)entry + entry->size + sizeof(entry->size).
 */
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

/* Type 1 means the region is normal, available RAM. */
#define MMAP_TYPE_AVAILABLE 1

uint32_t memory_get_kb(uint32_t address)
{
    struct multiboot_info *info =
        (struct multiboot_info *)address;

    /*
     * Multiboot flag 6: mmap_length/mmap_addr are valid.
     */
    if (info->flags & (1 << 6)) {
        uint64_t total_bytes = 0;

        uint32_t offset = 0;
        while (offset < info->mmap_length) {
            struct multiboot_mmap_entry *entry =
                (struct multiboot_mmap_entry *)
                (info->mmap_addr + offset);

            if (entry->type == MMAP_TYPE_AVAILABLE)
                total_bytes += entry->len;

            offset += entry->size + sizeof(entry->size);
        }

        return (uint32_t)(total_bytes / 1024);
    }

    /*
     * Multiboot flag 0: mem_lower/mem_upper are valid.
     *
     * Fallback for bootloaders that provide no memory map.
     * mem_upper is memory above 1 MiB, measured in KiB.
     */
    if (info->flags & 1)
        return info->mem_upper + 1024;

    return 0;
}