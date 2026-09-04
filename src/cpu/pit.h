#ifndef PIT_H
#define PIT_H

#include "types.h"

// Programmable Interval Timer (PIT)

// Initializes the PIT to the specified frequency (in Hz).
void pit_initialize(uint32_t frequency);

// Increments the PIT tick count.
void pit_tick(void);

// Returns the number of PIT ticks since initialization.
uint32_t pit_get_ticks(void);

#endif // PIT_H