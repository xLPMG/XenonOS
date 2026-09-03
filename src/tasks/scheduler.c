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
    thread_t *next = current->next;
    if (next == current)
        return;

    thread_t *old = current;
    current = next;
    context_switch((uint32_t *)&old->esp, (uint32_t)current->esp);
}
