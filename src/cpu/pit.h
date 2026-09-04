#ifndef PIT_H
#define PIT_H

#include "types.h"

// Programmable Interval Timer (PIT)

// Initializes the PIT to the specified frequency (in Hz).
void pit_initialize(uint32_t frequency);

#endif // PIT_H