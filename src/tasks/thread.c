#include "constants.h"
#include "slab.h"
#include "thread.h"
#include "scheduler.h"
#include "pmm.h"
#include "paging.h"
#include "pit.h"

static slab_cache_t thread_pool;
static int thread_pool_ready = 0;
static uint32_t next_stack_vaddr = STACK_REGION_BASE;

static inline uint32_t stack_frame_vaddr(uint32_t stack_vbase, int i)
{
    return stack_vbase + i * PAGE_SIZE;
}

// Runs the thread's real entry point. If it ever returns, there's no valid
// caller to return to (this thread was jumped into, never called), so park
// it here forever instead of letting it fall off the end.
static void thread_trampoline(void)
{
    __asm__ volatile("sti"); // sti = set interrupt flag
    thread_t *self = scheduler_current();
    self->entry();

    self->state = THREAD_TERMINATED;
    while (1)
        scheduler_yield();
}

// Builds a thread with prepared stack for context switching:
// 4 callee-saved register slots, then a return address that
// points at thread_trampoline so the first switch into this thread
// "returns" into it, which then calls the real entry point.
thread_t *thread_create(uint32_t id, void (*entry)(void))
{
    if (!thread_pool_ready)
    {
        // use tiny slab for thread header
        slab_cache_init(&thread_pool, sizeof(thread_t));
        thread_pool_ready = 1;
    }

    thread_t *thread = slab_alloc(&thread_pool);
    if (!thread)
        return 0;

    uint32_t stack_vbase = next_stack_vaddr;
    next_stack_vaddr += (STACK_PAGES + 1) * PAGE_SIZE; // +1 for guard page

    for (int i = 0; i < STACK_PAGES; i++)
    {
        uint32_t frame = pmm_alloc(); // allocate memory one frame at a time (physically separate frames)
        paging_map(stack_frame_vaddr(stack_vbase, i), frame, 3);
    }
    thread->mem = (void *)stack_vbase;
    uint8_t *stack_top = (uint8_t *)(stack_vbase + STACK_PAGES * PAGE_SIZE);

    stack_top -= POINTER_SIZE;
    *(void **)stack_top = (void *)thread_trampoline;

    // reserve space for the 4 callee-saved registers
    for (int i = 0; i < 4; i++)
    {
        stack_top -= POINTER_SIZE;
        *(void **)stack_top = 0;
    }

    thread->esp = stack_top;
    thread->id = id;
    thread->state = THREAD_READY;
    thread->next = 0;
    thread->entry = entry;

    return thread;
}

void thread_destroy(thread_t *thread)
{
    // Free the stack frames allocated for this thread
    uint32_t stack_vbase = (uint32_t)thread->mem;
    for (int i = 0; i < STACK_PAGES; i++)
    {
        uint32_t vaddr = stack_frame_vaddr(stack_vbase, i);
        uint32_t frame = paging_get_physical(vaddr);
        paging_unmap(vaddr);
        pmm_free(frame);
    }
    slab_free(&thread_pool, thread);
}

void thread_sleep(uint32_t ms)
{
    thread_t *current = scheduler_current();
    uint32_t ticks_to_sleep =
        (ms * TIMER_HZ + 1000 - 1) / 1000; // +1000 to round up
    current->wake_at = pit_get_ticks() + ticks_to_sleep;
    current->state = THREAD_BLOCKED;
    scheduler_yield();
}
