#include "pit.h"
#include "types.h"
#include "io_helper.h"
#include "constants.h"

static volatile uint32_t pit_ticks = 0;

void pit_initialize(uint32_t frequency)
{
    uint32_t divisor = PIT_FREQUENCY / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void pit_tick(void)
{
    pit_ticks++;
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}