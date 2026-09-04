#ifndef TSS_H
#define TSS_H

#include "types.h"

#define GDT_CODE_SELECTOR 0x08
#define GDT_DATA_SELECTOR 0x10
#define GDT_MAIN_TSS_SELECTOR 0x18
#define GDT_DF_TSS_SELECTOR 0x20

// Sets up a main TSS (landing spot for the CPU to save the interrupted
// task's state into) and a double-fault TSS pre-loaded with a known-good
// stack/entry point, then wires IDT vector 8 as a task gate to it. Must be
// called after paging_initialize() so the double-fault TSS captures the
// real CR3.
void tss_initialize(void);

#endif // TSS_H
