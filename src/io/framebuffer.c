#include "framebuffer.h"
#include "pci.h"
#include "paging.h"
#include "constants.h"
#include "multiboot.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_VIRT_WIDTH 6

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

// Signature range only real Bochs/QEMU virtual hardware reports back.
#define VBE_DISPI_ID_MIN 0xB0C0
#define VBE_DISPI_ID_MAX 0xB0C5

static struct framebuffer_info info;
static int initialized = 0;

static void dispi_write(uint16_t index, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(index), "Nd"((uint16_t)VBE_DISPI_IOPORT_INDEX));
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"((uint16_t)VBE_DISPI_IOPORT_DATA));
}

static uint16_t dispi_read(uint16_t index)
{
    uint16_t value;
    __asm__ volatile("outw %0, %1" : : "a"(index), "Nd"((uint16_t)VBE_DISPI_IOPORT_INDEX));
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"((uint16_t)VBE_DISPI_IOPORT_DATA));
    return value;
}

static void map_framebuffer(uint32_t physical_addr, uint32_t pitch, uint32_t width, uint32_t height, uint8_t bpp)
{
    uint32_t size = pitch * height;

    for (uint32_t offset = 0; offset < size && offset < FRAMEBUFFER_VIRTUAL_SIZE; offset += PAGE_SIZE)
        paging_map(FRAMEBUFFER_VIRTUAL_BASE + offset, physical_addr + offset, 3);

    info.addr = FRAMEBUFFER_VIRTUAL_BASE;
    info.pitch = pitch;
    info.width = width;
    info.height = height;
    info.bpp = bpp;
}

// Direct RGB framebuffer type, as opposed to indexed/palette or EGA text.
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1

static int try_grub_framebuffer(uint32_t multiboot_info_address)
{
    struct multiboot_tag_framebuffer *tag =
        (struct multiboot_tag_framebuffer *)multiboot_find_tag(multiboot_info_address, MULTIBOOT_TAG_FRAMEBUFFER);

    if (!tag)
        return 0;

    if (tag->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB ||
        tag->framebuffer_addr == 0 ||
        tag->framebuffer_width == 0 ||
        tag->framebuffer_height == 0)
        return 0;

    map_framebuffer((uint32_t)tag->framebuffer_addr, tag->framebuffer_pitch,
                     tag->framebuffer_width, tag->framebuffer_height, tag->framebuffer_bpp);
    return 1;
}

static int try_bochs_dispi(uint32_t width, uint32_t height, uint8_t bpp)
{
    uint16_t id = dispi_read(VBE_DISPI_INDEX_ID);
    if (id < VBE_DISPI_ID_MIN || id > VBE_DISPI_ID_MAX)
        return 0;

    uint8_t bus, device, function;
    if (!pci_find_vga_device(&bus, &device, &function))
        return 0;

    uint32_t physical_addr = pci_get_bar_address(bus, device, function, 0);

    dispi_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    dispi_write(VBE_DISPI_INDEX_XRES, (uint16_t)width);
    dispi_write(VBE_DISPI_INDEX_YRES, (uint16_t)height);
    dispi_write(VBE_DISPI_INDEX_BPP, bpp);
    dispi_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    map_framebuffer(physical_addr, width * (bpp / 8), width, height, bpp);
    return 1;
}

int framebuffer_initialize(uint32_t multiboot_info_address, uint32_t width, uint32_t height, uint8_t bpp)
{
    if (initialized)
        return 1;

    if (try_grub_framebuffer(multiboot_info_address) || try_bochs_dispi(width, height, bpp))
    {
        initialized = 1;
        return 1;
    }

    return 0;
}

const struct framebuffer_info *framebuffer_get_info(void)
{
    return &info;
}
