#include "test.h"
#include "thread.h"
#include "scheduler.h"
#include "constants.h"
#include "pit.h"

static void idle_entry(void)
{
}

static void test_create_returns_ready_thread(void)
{
    thread_t *t = thread_create(idle_entry);
    ASSERT(t != 0);
    ASSERT(t->state == THREAD_READY);
    ASSERT(t->mem != 0);
    ASSERT(t->esp != 0);

    thread_destroy(t);
}

static void test_distinct_threads_get_distinct_ids(void)
{
    thread_t *a = thread_create(idle_entry);
    thread_t *b = thread_create(idle_entry);

    ASSERT(a != 0 && b != 0);
    ASSERT(a->id != b->id);

    thread_destroy(a);
    thread_destroy(b);
}

static void test_distinct_threads_get_distinct_stacks(void)
{
    thread_t *a = thread_create(idle_entry);
    thread_t *b = thread_create(idle_entry);

    ASSERT(a != 0 && b != 0);
    ASSERT(a->mem != b->mem);

    thread_destroy(a);
    thread_destroy(b);
}

// Every thread's stack region is followed by an unmapped guard page before
// the next thread's stack starts, so two stacks should never sit closer
// together than that.
static void test_stacks_are_separated_by_a_guard_page(void)
{
    thread_t *a = thread_create(idle_entry);
    thread_t *b = thread_create(idle_entry);

    ASSERT(a != 0 && b != 0);

    uint32_t low = (uint32_t)a->mem < (uint32_t)b->mem ? (uint32_t)a->mem : (uint32_t)b->mem;
    uint32_t high = (uint32_t)a->mem < (uint32_t)b->mem ? (uint32_t)b->mem : (uint32_t)a->mem;

    ASSERT(high - low >= (DEFAULT_STACK_PAGES + 1) * PAGE_SIZE);

    thread_destroy(a);
    thread_destroy(b);
}

static volatile int ran_flag;

static void flag_setting_entry(void)
{
    ran_flag = 1;
}

static void test_scheduled_thread_actually_runs(void)
{
    ran_flag = 0;

    thread_t *t = thread_create(flag_setting_entry);
    ASSERT(t != 0);
    scheduler_add(t);

    // Give the scheduler a bounded number of chances to switch to it and
    // run its entry point, instead of looping forever if something's wrong.
    for (int i = 0; i < 20 && !ran_flag; i++)
        scheduler_yield();

    ASSERT(ran_flag == 1);
}

static void test_sleep_blocks_until_wake_at(void)
{
    uint32_t ms = 50;
    uint32_t ticks_to_sleep = (ms * TIMER_HZ + 1000 - 1) / 1000;

    uint32_t before = pit_get_ticks();
    thread_sleep(ms);
    uint32_t after = pit_get_ticks();

    ASSERT((int32_t)(after - before) >= (int32_t)ticks_to_sleep);
}

static const test_case_t cases[] = {
    {"create_returns_ready_thread", test_create_returns_ready_thread},
    {"distinct_threads_get_distinct_ids", test_distinct_threads_get_distinct_ids},
    {"distinct_threads_get_distinct_stacks", test_distinct_threads_get_distinct_stacks},
    {"stacks_are_separated_by_a_guard_page", test_stacks_are_separated_by_a_guard_page},
    {"scheduled_thread_actually_runs", test_scheduled_thread_actually_runs},
    {"sleep_blocks_until_wake_at", test_sleep_blocks_until_wake_at},
};

void test_suite_thread(void)
{
    test_run_suite("thread", cases, sizeof(cases) / sizeof(cases[0]));
}
