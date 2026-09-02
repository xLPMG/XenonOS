#include "pmm.h"
#include "constants.h"
#include "multiboot.h"

// Kernel start and end addresses provided by the linker script (linker.ld)
extern uint32_t kernel_start;
extern uint32_t kernel_end;

static uint32_t bitmap[BITMAP_SIZE];

static void set_frame(uint32_t frame)
{
    bitmap[frame / 32] |= (1 << (frame % 32));
}

static void clear_frame(uint32_t frame)
{
    bitmap[frame / 32] &= ~(1 << (frame % 32));
}

static int test_frame(uint32_t frame)
{
    return bitmap[frame / 32] & (1 << (frame % 32));
}

static uint32_t first_free_frame(void)
{
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
    {
        if (bitmap[i] != 0xFFFFFFFF)
        {
            for (uint32_t j = 0; j < 32; j++)
            {
                if (!(bitmap[i] & (1 << j)))
                {
                    return i * 32 + j;
                }
            }
        }
    }
    return (uint32_t)-1; // no free frame
}

// Mark every frame overlapping [base, base + length) as FREE.
// The base is rounded up and the end down so only fully covered frames are freed.
static void free_region(uint64_t base, uint64_t length)
{
    uint64_t first = (base + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last = (base + length) / PAGE_SIZE;

    for (uint64_t frame = first; frame < last && frame < FRAME_COUNT; frame++)
    {
        clear_frame((uint32_t)frame);
    }
}

// Mark every frame overlapping [base, base + length) as USED.
// The base is rounded down and the end up so partially covered frames stay reserved.
static void reserve_region(uint64_t base, uint64_t length)
{
    uint64_t first = base / PAGE_SIZE;
    uint64_t last = (base + length + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t frame = first; frame < last && frame < FRAME_COUNT; frame++)
    {
        set_frame((uint32_t)frame);
    }
}

void pmm_initialize(uint32_t multiboot_info_address)
{
    // 1) Mark everything as USED
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
    {
        bitmap[i] = 0xFFFFFFFF;
    }

    // 2) Read the Multiboot memory map and mark usable regions as FREE
    struct multiboot_info *info =
        (struct multiboot_info *)multiboot_info_address;

    if (info->flags & MULTIBOOT_FLAG_MMAP)
    {
        uint32_t offset = 0;
        while (offset < info->mmap_length)
        {
            struct multiboot_mmap_entry *entry =
                (struct multiboot_mmap_entry *)(info->mmap_addr + offset);

            if (entry->type == MMAP_TYPE_AVAILABLE)
                free_region(entry->addr, entry->len);

            offset += entry->size + sizeof(entry->size);
        }
    }

    // 3) Mark the low 1 MiB (BIOS/IVT/VGA) and the kernel image as USED
    reserve_region(0, 0x100000);

    uint32_t kstart = (uint32_t)&kernel_start;
    uint32_t kend = (uint32_t)&kernel_end;
    reserve_region(kstart, kend - kstart);
}

uint32_t pmm_alloc(void)
{
    // 1) find free frame
    uint32_t frame = first_free_frame();
    if (frame == (uint32_t)-1)
    {
        return (uint32_t)-1;
    }

    // 2) mark it used
    set_frame(frame);

    // 3) return physical address
    return frame * PAGE_SIZE;
}

void pmm_free(uint32_t address)
{
    // 1) convert address to frame number
    uint32_t frame = address / PAGE_SIZE;
    // 2) mark it free
    clear_frame(frame);
}