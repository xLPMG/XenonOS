#include "terminal.h"
#include <constants.h>
#include "io_helper.h"

static volatile unsigned short *video =
    (volatile unsigned short *)0xB8000;

static int row = 0;
static int column = 0;

static unsigned short entry(char c)
{
    return (unsigned short)((0x07 << 8) | (unsigned char)c);
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

    clear();
    update_cursor();
}

void terminal_putchar(char c)
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

void terminal_backspace(void)
{
    if (column == 0 && row == 0)
        return;

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
}

void terminal_write(const char *str)
{
    while (*str)
        terminal_putchar(*str++);
}