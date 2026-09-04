#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "thread.h"

// Registers the boot context (kmain) as "thread 0" so scheduler_yield has
// somewhere to save its esp before switching to the first real thread.
void scheduler_initialize(void);

// Adds a thread to the ready queue.
void scheduler_add(thread_t *thread);

// Yields execution from the current thread to the next thread in the ready queue.
void scheduler_yield(void);

// Returns the thread that's currently running.
thread_t *scheduler_current(void);

#endif // SCHEDULER_H
