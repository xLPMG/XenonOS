#include "test.h"
#include "thread.h"
#include "scheduler.h"
#include "constants.h"

static void idle_entry(void)
{
}

static void test_create_returns_ready_thread(void)
{
    thread_t *t = thread_create(100, idle_entry);
    ASSERT(t != 0);
    ASSERT(t->id == 100);
    ASSERT(t->state == THREAD_READY);
    ASSERT(t->mem != 0);
    ASSERT(t->esp != 0);

    thread_destroy(t);
}

static void test_distinct_threads_get_distinct_stacks(void)
{
    thread_t *a = thread_create(1, idle_entry);
    thread_t *b = thread_create(2, idle_entry);

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
    thread_t *a = thread_create(1, idle_entry);
    thread_t *b = thread_create(2, idle_entry);

    ASSERT(a != 0 && b != 0);

    uint32_t low = (uint32_t)a->mem < (uint32_t)b->mem ? (uint32_t)a->mem : (uint32_t)b->mem;
    uint32_t high = (uint32_t)a->mem < (uint32_t)b->mem ? (uint32_t)b->mem : (uint32_t)a->mem;

    ASSERT(high - low >= (STACK_PAGES + 1) * PAGE_SIZE);

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

    thread_t *t = thread_create(200, flag_setting_entry);
    ASSERT(t != 0);
    scheduler_add(t);

    // Give the scheduler a bounded number of chances to switch to it and
    // run its entry point, instead of looping forever if something's wrong.
    for (int i = 0; i < 20 && !ran_flag; i++)
        scheduler_yield();

    ASSERT(ran_flag == 1);
}

static const test_case_t cases[] = {
    {"create_returns_ready_thread", test_create_returns_ready_thread},
    {"distinct_threads_get_distinct_stacks", test_distinct_threads_get_distinct_stacks},
    {"stacks_are_separated_by_a_guard_page", test_stacks_are_separated_by_a_guard_page},
    {"scheduled_thread_actually_runs", test_scheduled_thread_actually_runs},
};

void test_suite_thread(void)
{
    test_run_suite("thread", cases, sizeof(cases) / sizeof(cases[0]));
}
