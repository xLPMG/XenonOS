#ifndef TEST_H
#define TEST_H

// Set by ASSERT on failure so test_run_suite can report it.
extern int test_current_failed;
extern const char *test_current_failure_expr;
extern const char *test_current_failure_file;
extern int test_current_failure_line;

#define ASSERT(cond)                                    \
    do                                                  \
    {                                                   \
        if (!(cond))                                    \
        {                                               \
            test_current_failed = 1;                    \
            test_current_failure_expr = #cond;           \
            test_current_failure_file = __FILE__;        \
            test_current_failure_line = __LINE__;        \
            return;                                     \
        }                                               \
    } while (0)

typedef void (*test_fn)(void);

typedef struct
{
    const char *name;
    test_fn fn;
} test_case_t;

void test_run_suite(const char *suite_name, const test_case_t *cases, int count);
void test_run_all(void);

#endif
