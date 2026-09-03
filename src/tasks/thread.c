#include "constants.h"
#include "slab.h"
#include "thread.h"
#include "pmm.h"
#include "paging.h"

static slab_cache_t thread_pool;
static int thread_pool_ready = 0;

// Builds a thread with prepared stack for context switching:
// 4 callee-saved register slots, then a return address that
// points at `entry` so the first switch into this thread "returns" into it.
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

    uint32_t stack_frame = pmm_alloc();
    if (stack_frame == (uint32_t)-1)
        return 0;

    // identity-map the stack frame so the CPU can access it directly
    paging_map(stack_frame, stack_frame, 3);
    thread->mem = (void *)stack_frame;

    // stack grows downwards!
    uint8_t *stack_top = (uint8_t *)stack_frame + STACK_SIZE;

    stack_top -= POINTER_SIZE;
    *(void **)stack_top = (void *)entry;

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

    return thread;
}

