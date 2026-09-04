#include "terminal.h"
#include <constants.h>
#include "io_helper.h"
#include "spinlock.h"
#include "string.h"
#include <stdarg.h>

static volatile unsigned short *video =
    (volatile unsigned short *)0xB8000;

static int row = 0;
static int column = 0;
static unsigned char current_color = (VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY;
static spinlock_t terminal_lock;

static unsigned short entry(char c)
{
    return (unsigned short)((current_color << 8) | (unsigned char)c);
}

static void update_cursor(void)
{
    unsigned short position =
        row * VGA_WIDTH + column;

    // VGA cursor position low byte
    outb(0x3D4, 0x0F);
    outb(0x3D5, position & 0xFF);

    // VGA cursor position high byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, position >> 8);
}

static void clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        video[i] = entry(' ');

    update_cursor();
}

static void scroll(void)
{
    if (row < VGA_HEIGHT)
        return;

    for (int y = 1; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            video[(y - 1) * VGA_WIDTH + x] =
                video[y * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++)
        video[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            entry(' ');

    row = VGA_HEIGHT - 1;
}

void terminal_initialize(void)
{
    row = 0;
    column = 0;

    spinlock_initialize(&terminal_lock);

    clear();
    update_cursor();
}

// Assumes terminal_lock is already held by the caller.
static void terminal_putchar_locked(char c)
{
    if (c == '\n')
    {
        column = 0;
        row++;

        scroll();
        update_cursor();
        return;
    }

    video[row * VGA_WIDTH + column] = entry(c);

    column++;

    if (column >= VGA_WIDTH)
    {
        column = 0;
        row++;

        scroll();
    }

    update_cursor();
}

void terminal_putchar(char c)
{
    uint32_t flags = spinlock_acquire(&terminal_lock);
    terminal_putchar_locked(c);
    spinlock_release(&terminal_lock, flags);
}

void terminal_backspace(void)
{
    uint32_t flags = spinlock_acquire(&terminal_lock);

    if (column == 0 && row == 0)
    {
        spinlock_release(&terminal_lock, flags);
        return;
    }

    if (column > 0)
    {
        column--;
    }
    else
    {
        row--;
        column = VGA_WIDTH - 1;
    }

    video[row * VGA_WIDTH + column] = entry(' ');

    update_cursor();
    spinlock_release(&terminal_lock, flags);
}

void terminal_write(const char *str)
{
    while (*str)
        terminal_putchar(*str++);
}

void terminal_set_color(enum vga_color fg, enum vga_color bg)
{
    uint32_t flags = spinlock_acquire(&terminal_lock);
    current_color = (unsigned char)((bg << 4) | (fg & 0x0F));
    spinlock_release(&terminal_lock, flags);
}

void terminal_write_colored(const char *str, enum vga_color fg, enum vga_color bg)
{
    uint32_t flags = spinlock_acquire(&terminal_lock);

    unsigned char previous_color = current_color;
    current_color = (unsigned char)((bg << 4) | (fg & 0x0F));

    while (*str)
        terminal_putchar_locked(*str++);

    current_color = previous_color;

    spinlock_release(&terminal_lock, flags);
}

// Assumes terminal_lock is already held by the caller.
static void write_formatted_locked(const char *format, va_list args)
{
    char number[16];

    while (*format)
    {
        if (*format != '%')
        {
            terminal_putchar_locked(*format++);
            continue;
        }

        format++;
        switch (*format)
        {
        case 'u':
            itoa(va_arg(args, unsigned int), number);
            for (char *p = number; *p; p++)
                terminal_putchar_locked(*p);
            break;
        case 'x':
            itoa_hex(va_arg(args, unsigned int), number);
            for (char *p = number; *p; p++)
                terminal_putchar_locked(*p);
            break;
        case 's':
            for (const char *s = va_arg(args, const char *); *s; s++)
                terminal_putchar_locked(*s);
            break;
        case 'c':
            terminal_putchar_locked((char)va_arg(args, int));
            break;
        case '%':
            terminal_putchar_locked('%');
            break;
        default:
            terminal_putchar_locked('%');
            terminal_putchar_locked(*format);
            break;
        }

        format++;
    }
}

void terminal_writef(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    uint32_t flags = spinlock_acquire(&terminal_lock);
    write_formatted_locked(format, args);
    spinlock_release(&terminal_lock, flags);

    va_end(args);
}

void terminal_writef_colored(enum vga_color fg, enum vga_color bg, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    uint32_t flags = spinlock_acquire(&terminal_lock);

    unsigned char previous_color = current_color;
    current_color = (unsigned char)((bg << 4) | (fg & 0x0F));

    write_formatted_locked(format, args);

    current_color = previous_color;

    spinlock_release(&terminal_lock, flags);

    va_end(args);
}