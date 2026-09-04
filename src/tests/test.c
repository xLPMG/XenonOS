#include "test.h"
#include "terminal.h"
#include "string.h"

int test_current_failed;
const char *test_current_failure_expr;
const char *test_current_failure_file;
int test_current_failure_line;

static int total_tests;
static int total_failures;

extern void test_suite_string(void);
extern void test_suite_pmm(void);
extern void test_suite_paging(void);
extern void test_suite_slab(void);
extern void test_suite_heap(void);
extern void test_suite_spinlock(void);
extern void test_suite_thread(void);

void test_run_suite(const char *suite_name, const test_case_t *cases, int count)
{
    for (int i = 0; i < count; i++)
    {
        total_tests++;
        test_current_failed = 0;

        cases[i].fn();

        if (test_current_failed)
        {
            total_failures++;

            terminal_writef_colored(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK, "[FAIL] %s / %s\n       %s at %s:%u\n",
                                    suite_name, cases[i].name, test_current_failure_expr,
                                    test_current_failure_file, (unsigned int)test_current_failure_line);
        }
    }
}

void test_run_all(void)
{
    total_tests = 0;
    total_failures = 0;

    terminal_write("Running tests...\n");

    test_suite_string();
    test_suite_pmm();
    test_suite_paging();
    test_suite_slab();
    test_suite_heap();
    test_suite_spinlock();
    test_suite_thread();

    terminal_write("\n");

    if (total_failures == 0)
    {
        terminal_write_colored("Test was successful. Everything works as intended :)\n",
                               VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    }
    else
    {
        terminal_writef_colored(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK, "%u of %u tests failed.\n",
                                (unsigned int)total_failures, (unsigned int)total_tests);
    }

    terminal_write("\n> ");
}
