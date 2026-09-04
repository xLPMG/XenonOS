#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

// Debug output channel independent of VGA/framebuffer state; useful whenever
// the video mode doesn't have a working text console attached.
void serial_initialize(void);
void serial_write(const char *str);

// Minimal printf-style formatter supporting %u, %x, %s, %c, %%
void serial_writef(const char *format, ...);

#endif // SERIAL_H
