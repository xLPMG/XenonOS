#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "memory.h"

// Multiboot2 boot information: a fixed 8-byte header followed by a stream of
// tags, each 8-byte aligned. See boot.asm for the matching header we send to
// request these from GRUB.
struct multiboot_info
{
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot_tag
{
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

#define MULTIBOOT_TAG_END 0
#define MULTIBOOT_TAG_BASIC_MEMORY 4
#define MULTIBOOT_TAG_MMAP 6
#define MULTIBOOT_TAG_FRAMEBUFFER 8

struct multiboot_tag_basic_memory
{
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed));

struct multiboot_tag_mmap
{
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    // followed by (size - 16) / entry_size entries
} __attribute__((packed));

// One entry of the multiboot memory map.
// Next entry starts at (uint8_t *)entry + entry_size (from the owning tag).
struct multiboot_mmap_entry
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

// Type 1 means the region is normal, available RAM
#define MMAP_TYPE_AVAILABLE 1

struct multiboot_tag_framebuffer
{
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved2;
    // color info follows, depends on framebuffer_type - unused here
} __attribute__((packed));

// Finds the first tag of the given type in the tag list, or NULL if absent.
struct multiboot_tag *multiboot_find_tag(uint32_t multiboot_info_address, uint32_t type);

#endif
