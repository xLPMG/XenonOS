#include "serial.h"
#include "io_helper.h"
#include "string.h"
#include <stdarg.h>

#define COM1 0x3F8

void serial_initialize(void)
{
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // enable DLAB to set baud rate divisor
    outb(COM1 + 0, 0x03); // divisor low byte: 38400 baud
    outb(COM1 + 1, 0x00); // divisor high byte
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // enable FIFO, clear it, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs disabled, RTS/DSR set
}

static int transmit_empty(void)
{
    return inb(COM1 + 5) & 0x20;
}

static void serial_putc(char c)
{
    while (!transmit_empty())
        ;
    outb(COM1, (uint8_t)c);
}

void serial_write(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
            serial_putc('\r');
        serial_putc(*str++);
    }
}

void serial_writef(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char number[16];

    while (*format)
    {
        if (*format != '%')
        {
            serial_write((const char[]){*format++, '\0'});
            continue;
        }

        format++;
        switch (*format)
        {
        case 'u':
            itoa(va_arg(args, unsigned int), number);
            serial_write(number);
            break;
        case 'x':
            itoa_hex(va_arg(args, unsigned int), number);
            serial_write(number);
            break;
        case 's':
            serial_write(va_arg(args, const char *));
            break;
        case 'c':
            serial_write((const char[]){(char)va_arg(args, int), '\0'});
            break;
        case '%':
            serial_write("%");
            break;
        default:
            serial_write((const char[]){'%', *format, '\0'});
            break;
        }

        format++;
    }

    va_end(args);
}
