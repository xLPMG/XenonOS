#ifndef THREAD_H
#define THREAD_H

#include "types.h"

typedef struct thread thread_t;

enum thread_state {
  THREAD_RUNNING,
  THREAD_READY,
  THREAD_BLOCKED,
  THREAD_TERMINATED
};

// Represents a thread in the system, including its state, stack pointer,
// and linkage for the ready queue.
struct thread {
  uint32_t id;
  void *esp;               // stack pointer
  void *mem;               // virtual base address of the thread's memory region
  enum thread_state state; // current state of the thread
  struct thread *next;     // pointer to the next thread in the list
  void (*entry)(void);     // the thread's actual entry point
  uint32_t wake_at;        // tick count at which the thread should wake up
};

// Saves the currently running context's esp into *old_esp, then switches
// to new_esp and resumes whatever was previously saved there.
void context_switch(uint32_t *old_esp, uint32_t new_esp);

// Creates a new thread with the given id and entry point.
// The new thread will have its own stack and be ready to run.
thread_t *thread_create(uint32_t id, void (*entry)(void));

// Frees a terminated thread's stack and header. Must only be called on a
// thread that isn't currently running (i.e. not `current`).
void thread_destroy(thread_t *thread);

// Puts the current thread to sleep until the specified number of milliseconds have passed.
void thread_sleep(uint32_t ms);

#endif // THREAD_H