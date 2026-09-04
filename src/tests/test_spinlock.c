#include "test.h"
#include "spinlock.h"

#define EFLAGS_IF (1u << 9)

static inline uint32_t read_eflags(void)
{
    uint32_t flags;
    __asm__ volatile("pushfl; popl %0" : "=r"(flags));
    return flags;
}

static void test_initialize_starts_unlocked(void)
{
    spinlock_t lock;
    spinlock_initialize(&lock);

    ASSERT(lock.locked == 0);
}

static void test_acquire_locks_and_disables_interrupts(void)
{
    spinlock_t lock;
    spinlock_initialize(&lock);

    __asm__ volatile("sti"); // known starting state: interrupts enabled

    uint32_t flags = spinlock_acquire(&lock);

    ASSERT(lock.locked == 1);
    ASSERT((read_eflags() & EFLAGS_IF) == 0);

    spinlock_release(&lock, flags);
}

static void test_release_unlocks_and_restores_enabled_interrupts(void)
{
    spinlock_t lock;
    spinlock_initialize(&lock);

    __asm__ volatile("sti");

    uint32_t flags = spinlock_acquire(&lock);
    spinlock_release(&lock, flags);

    ASSERT(lock.locked == 0);
    ASSERT((read_eflags() & EFLAGS_IF) != 0);
}

static void test_release_preserves_prior_disabled_interrupts(void)
{
    spinlock_t lock;
    spinlock_initialize(&lock);

    uint32_t was_enabled = read_eflags() & EFLAGS_IF;
    __asm__ volatile("cli"); // simulate being called from an already-cli'd context

    uint32_t flags = spinlock_acquire(&lock);
    spinlock_release(&lock, flags);

    // Release must restore exactly what was there before acquire, not just
    // blindly re-enable interrupts.
    ASSERT((read_eflags() & EFLAGS_IF) == 0);

    if (was_enabled)
        __asm__ volatile("sti"); // restore the real pre-test state
}

static const test_case_t cases[] = {
    {"initialize_starts_unlocked", test_initialize_starts_unlocked},
    {"acquire_locks_and_disables_interrupts", test_acquire_locks_and_disables_interrupts},
    {"release_unlocks_and_restores_enabled_interrupts", test_release_unlocks_and_restores_enabled_interrupts},
    {"release_preserves_prior_disabled_interrupts", test_release_preserves_prior_disabled_interrupts},
};

void test_suite_spinlock(void)
{
    test_run_suite("spinlock", cases, sizeof(cases) / sizeof(cases[0]));
}
