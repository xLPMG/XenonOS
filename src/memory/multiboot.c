#include "multiboot.h"

struct multiboot_tag *multiboot_find_tag(uint32_t multiboot_info_address, uint32_t type)
{
    struct multiboot_info *info = (struct multiboot_info *)multiboot_info_address;

    uint8_t *ptr = (uint8_t *)multiboot_info_address + sizeof(struct multiboot_info);
    uint8_t *end = (uint8_t *)multiboot_info_address + info->total_size;

    while (ptr < end)
    {
        struct multiboot_tag *tag = (struct multiboot_tag *)ptr;

        if (tag->type == MULTIBOOT_TAG_END)
            return (struct multiboot_tag *)0;

        if (tag->type == type)
            return tag;

        // Tags are 8-byte aligned; size does not include this padding.
        ptr += (tag->size + 7) & ~7u;
    }

    return (struct multiboot_tag *)0;
}
