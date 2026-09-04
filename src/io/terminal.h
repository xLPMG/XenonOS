#ifndef TERMINAL_H
#define TERMINAL_H

// Standard VGA text-mode 4-bit color palette.
enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14, // aka yellow
    VGA_COLOR_WHITE = 15,
};

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_backspace(void);
void terminal_write(const char *str);

// Sets the foreground/background color used by subsequent writes.
void terminal_set_color(enum vga_color fg, enum vga_color bg);

// Atomically writes str in the given color, then restores the previous
// color, all under one lock acquisition so no other thread's output (or
// color change) can interleave in between.
void terminal_write_colored(const char *str, enum vga_color fg, enum vga_color bg);

// printf-style formatted write, atomic for the whole call. Supported
// specifiers: %u (decimal), %x (hex), %s (string), %c (char), %% (literal).
void terminal_writef(const char *format, ...);
void terminal_writef_colored(enum vga_color fg, enum vga_color bg, const char *format, ...);

#endif