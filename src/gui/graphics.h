#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

// All functions are no-ops if called before a successful framebuffer_initialize().

void gfx_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

// Draws a single glyph from the built-in 8x8 font
void gfx_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void gfx_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg);

#endif // GRAPHICS_H
