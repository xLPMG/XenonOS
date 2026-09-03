#include "test.h"
#include "string.h"

static void test_strcmp_equal(void)
{
    ASSERT(strcmp("abc", "abc") == 0);
}

static void test_strcmp_different(void)
{
    ASSERT(strcmp("abc", "abd") != 0);
}

static void test_strlen(void)
{
    ASSERT(strlen("hello") == 5);
    ASSERT(strlen("") == 0);
}

static void test_memcpy(void)
{
    char src[] = "hi";
    char dst[3] = {0};

    memcpy(dst, src, 3);

    ASSERT(dst[0] == 'h');
    ASSERT(dst[1] == 'i');
    ASSERT(dst[2] == '\0');
}

static void test_memset(void)
{
    char buf[4];

    memset(buf, 'x', sizeof(buf));

    ASSERT(buf[0] == 'x');
    ASSERT(buf[3] == 'x');
}

static void test_memcmp(void)
{
    ASSERT(memcmp("abc", "abc", 3) == 0);
    ASSERT(memcmp("abc", "abd", 3) != 0);
}

static void test_itoa(void)
{
    char buf[16];

    itoa(0, buf);
    ASSERT(strcmp(buf, "0") == 0);

    itoa(123, buf);
    ASSERT(strcmp(buf, "123") == 0);
}

static void test_itoa_hex(void)
{
    char buf[16];

    itoa_hex(0, buf);
    ASSERT(strcmp(buf, "0x0") == 0);

    itoa_hex(0xABC, buf);
    ASSERT(strcmp(buf, "0xabc") == 0);
}

static const test_case_t cases[] = {
    {"strcmp_equal", test_strcmp_equal},
    {"strcmp_different", test_strcmp_different},
    {"strlen", test_strlen},
    {"memcpy", test_memcpy},
    {"memset", test_memset},
    {"memcmp", test_memcmp},
    {"itoa", test_itoa},
    {"itoa_hex", test_itoa_hex},
};

void test_suite_string(void)
{
    test_run_suite("string", cases, sizeof(cases) / sizeof(cases[0]));
}
