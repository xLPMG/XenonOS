#include "memory.h"
#include "multiboot.h"

uint32_t memory_get_kb(uint32_t address)
{
    struct multiboot_tag_mmap *mmap_tag =
        (struct multiboot_tag_mmap *)multiboot_find_tag(address, MULTIBOOT_TAG_MMAP);

    if (mmap_tag)
    {
        uint64_t total_bytes = 0;

        uint8_t *entry_ptr = (uint8_t *)mmap_tag + sizeof(struct multiboot_tag_mmap);
        uint8_t *end = (uint8_t *)mmap_tag + mmap_tag->size;

        while (entry_ptr < end)
        {
            struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)entry_ptr;

            if (entry->type == MMAP_TYPE_AVAILABLE)
                total_bytes += entry->len;

            entry_ptr += mmap_tag->entry_size;
        }

        return (uint32_t)(total_bytes / 1024);
    }

    // Fallback for bootloaders that provide no memory map.
    // mem_upper is memory above 1 MiB, measured in KiB.
    struct multiboot_tag_basic_memory *mem_tag =
        (struct multiboot_tag_basic_memory *)multiboot_find_tag(address, MULTIBOOT_TAG_BASIC_MEMORY);

    if (mem_tag)
        return mem_tag->mem_upper + 1024;

    return 0;
}