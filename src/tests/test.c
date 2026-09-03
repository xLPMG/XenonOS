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

void test_run_suite(const char *suite_name, const test_case_t *cases, int count)
{
    for (int i = 0; i < count; i++)
    {
        total_tests++;
        test_current_failed = 0;

        cases[i].fn();

        if (test_current_failed)
        {
            char line[16];

            total_failures++;

            terminal_write("[FAIL] ");
            terminal_write(suite_name);
            terminal_write(" / ");
            terminal_write(cases[i].name);
            terminal_write("\n       ");
            terminal_write(test_current_failure_expr);
            terminal_write(" at ");
            terminal_write(test_current_failure_file);
            terminal_write(":");
            itoa((unsigned int)test_current_failure_line, line);
            terminal_write(line);
            terminal_write("\n");
        }
    }
}

void test_run_all(void)
{
    char count[16];

    total_tests = 0;
    total_failures = 0;

    terminal_write("Running tests...\n");

    test_suite_string();
    test_suite_pmm();
    test_suite_paging();

    terminal_write("\n");

    if (total_failures == 0)
    {
        terminal_write("Test was successful. Everything works as intended :)\n");
    }
    else
    {
        itoa((unsigned int)total_failures, count);
        terminal_write(count);
        terminal_write(" of ");
        itoa((unsigned int)total_tests, count);
        terminal_write(count);
        terminal_write(" tests failed.\n");
    }

    terminal_write("\n> ");
}
