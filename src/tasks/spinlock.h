#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

typedef struct spinlock spinlock_t;

struct spinlock
{
    volatile uint32_t locked;
};

void spinlock_initialize(spinlock_t *lock);
uint32_t spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock, uint32_t flags);

#endif // SPINLOCK_H