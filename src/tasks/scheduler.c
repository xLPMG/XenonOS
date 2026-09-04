#include "scheduler.h"
#include "pit.h"
#include "types.h"
#include "spinlock.h"

static thread_t boot_thread;
static thread_t *current = 0;
static spinlock_t scheduler_lock;

// Runs whenever no real thread is ready, so scheduler_yield always has a
// valid candidate to switch to instead of giving up on a BLOCKED current.
static void idle_loop(void)
{
    while (1)
        __asm__ volatile("hlt");
}

void scheduler_initialize(void)
{
    boot_thread.id = 0;
    boot_thread.state = THREAD_RUNNING;
    boot_thread.next = &boot_thread;

    current = &boot_thread;
    boot_thread.next = &boot_thread;

    spinlock_initialize(&scheduler_lock);

    thread_t *idle_thread = thread_create_sized(idle_loop, 1);
    scheduler_add(idle_thread);
}

void scheduler_add(thread_t *thread)
{
    uint32_t flags = spinlock_acquire(&scheduler_lock);
    thread->next = current->next;
    current->next = thread;
    spinlock_release(&scheduler_lock, flags);
}

void scheduler_yield(void)
{
    uint32_t flags = spinlock_acquire(&scheduler_lock);

    thread_t *prev = current;
    thread_t *candidate = current->next;

    while (candidate != current)
    {
        if (candidate->state == THREAD_TERMINATED)
        {
            thread_t *dead = candidate;
            candidate = candidate->next;
            prev->next = candidate;
            thread_destroy(dead);
        }
        // weird comparison style in case tick counter wraps around
        else if (candidate->state == THREAD_BLOCKED && (int32_t)(pit_get_ticks() - candidate->wake_at) < 0)
        {
            // still asleep - skip over this
            prev = candidate;
            candidate = candidate->next;
        }
        else
        {
            // candidate was found
            if (candidate->state == THREAD_BLOCKED)
                candidate->state = THREAD_READY;
            break;
        }
    }

    if (candidate == current)
    {
        spinlock_release(&scheduler_lock, flags);
        return;
    }

    thread_t *old = current;
    current = candidate;

    // Release before switching stacks: this thread won't run again to
    // release it later, so holding it across the switch would deadlock
    // the very next thread that tries to acquire it.
    spinlock_release(&scheduler_lock, flags);

    context_switch((uint32_t *)&old->esp, (uint32_t)current->esp);
}

thread_t *scheduler_current(void)
{
    return current;
}
