#include "scheduler.h"

static thread_t boot_thread;
static thread_t *current = 0;

void scheduler_initialize(void)
{
    boot_thread.id = 0;
    boot_thread.state = THREAD_RUNNING;
    boot_thread.next = &boot_thread;

    current = &boot_thread;
    boot_thread.next = &boot_thread;
}

void scheduler_add(thread_t *thread)
{
    thread->next = current->next;
    current->next = thread;
}

void scheduler_yield(void)
{
    thread_t *prev = current;
    thread_t *candidate = current->next;

    // Reclaim any terminated threads we pass along the way - safe here
    // since none of them are the one currently executing this code.
    while (candidate != current && candidate->state == THREAD_TERMINATED)
    {
        thread_t *dead = candidate;
        candidate = candidate->next;

        prev->next = candidate;
        thread_destroy(dead);
    }

    if (candidate == current)
        return;

    thread_t *old = current;
    current = candidate;
    context_switch((uint32_t *)&old->esp, (uint32_t)current->esp);
}

thread_t *scheduler_current(void)
{
    return current;
}
