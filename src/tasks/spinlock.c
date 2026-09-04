#include "spinlock.h"

// Reads EFLAGS, disables interrupts, returns the flags as they were before.
static inline uint32_t save_and_disable_interrupts(void)
{
    uint32_t flags;
    __asm__ volatile("pushfl; popl %0; cli" : "=r"(flags)::"memory");
    return flags;
}

// Restores EFLAGS (including IF) exactly as previously saved.
static inline void restore_interrupts(uint32_t flags)
{
    __asm__ volatile("pushl %0; popfl" ::"r"(flags) : "memory", "cc");
}

// Atomically swaps *ptr with val and returns the old value.
// xchg is always atomic on x86, even without an explicit `lock` prefix -
// this is what actually protects against another core in the future.
static inline uint32_t atomic_xchg(volatile uint32_t *ptr, uint32_t val)
{
    uint32_t old;
    __asm__ volatile("xchg %0, %1" : "=r"(old), "+m"(*ptr) : "0"(val) : "memory");
    return old;
}

void spinlock_initialize(spinlock_t *lock)
{
    lock->locked = 0;
}

uint32_t spinlock_acquire(spinlock_t *lock)
{
    uint32_t flags = save_and_disable_interrupts();
    while (atomic_xchg(&lock->locked, 1) != 0)
        __asm__ volatile("pause");
    return flags;
}

void spinlock_release(spinlock_t *lock, uint32_t flags)
{
    atomic_xchg(&lock->locked, 0);
    restore_interrupts(flags);
}