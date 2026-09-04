#include "pit.h"
#include "types.h"
#include "io_helper.h"

void pit_initialize(uint32_t frequency)
{
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}