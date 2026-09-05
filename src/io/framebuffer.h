#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "types.h"

struct framebuffer_info
{
    uint32_t addr; // off-screen draw buffer (heap-allocated) - see framebuffer_present()
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
};

// Uses the framebuffer GRUB set up (BIOS VBE/UEFI GOP) if it provided a valid
// one, otherwise falls back to programming the emulated Bochs/QEMU "dispi"
// interface directly. Either way, maps the resulting linear framebuffer.
// Returns 1 on success, 0 if no framebuffer could be obtained either way.
int framebuffer_initialize(uint32_t multiboot_info_address, uint32_t width, uint32_t height, uint8_t bpp);

// Valid only after a successful framebuffer_initialize().
const struct framebuffer_info *framebuffer_get_info(void);

// Copies the off-screen draw buffer to the real hardware framebuffer in one
// shot. Call this after a batch of gfx_* drawing is done, so the screen never
// shows a partially-drawn frame (which looks like flicker).
void framebuffer_present(void);

#endif // FRAMEBUFFER_H
