#include "memory.h"
#include "multiboot.h"

uint32_t memory_get_kb(uint32_t address)
{
    struct multiboot_info *info =
        (struct multiboot_info *)address;

    // Multiboot flag 6: mmap_length/mmap_addr are valid
    if (info->flags & MULTIBOOT_FLAG_MMAP)
    {
        uint64_t total_bytes = 0;

        uint32_t offset = 0;
        while (offset < info->mmap_length)
        {
            struct multiboot_mmap_entry *entry =
                (struct multiboot_mmap_entry *)(info->mmap_addr + offset);

            if (entry->type == MMAP_TYPE_AVAILABLE)
                total_bytes += entry->len;

            offset += entry->size + sizeof(entry->size);
        }

        return (uint32_t)(total_bytes / 1024);
    }

    // Multiboot flag 0: mem_lower/mem_upper are valid.
    // Fallback for bootloaders that provide no memory map.
    // mem_upper is memory above 1 MiB, measured in KiB.
    if (info->flags & MULTIBOOT_FLAG_MEM)
        return info->mem_upper + 1024;

    return 0;
}