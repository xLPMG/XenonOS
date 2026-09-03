#include "test.h"
#include "paging.h"

static void test_map_and_unmap(void)
{
    uint32_t virtual_address = 0x00300000;
    uint32_t physical_address = 0x00301000;

    paging_map(virtual_address, physical_address, 3);
    ASSERT(paging_get_physical(virtual_address) == physical_address);

    paging_unmap(virtual_address);
    ASSERT(paging_get_physical(virtual_address) == 0);

    paging_map(virtual_address, virtual_address, 3); // restore identity mapping
}

static void test_hardware_translation(void)
{
    uint32_t virtual_address = 0x00300000;
    uint32_t physical_address = 0x00301000;

    *(volatile uint32_t *)physical_address = 0x12345678;

    paging_map(virtual_address, physical_address, 3);
    ASSERT(*(volatile uint32_t *)virtual_address == 0x12345678);

    paging_unmap(virtual_address);
    paging_map(virtual_address, virtual_address, 3); // restore identity mapping
}

static void test_map_beyond_first_4mb(void)
{
    uint32_t virtual_address = 0x01100000;
    uint32_t physical_address = 0x00302000;

    *(volatile uint32_t *)physical_address = 0x0BADF00D;

    paging_map(virtual_address, physical_address, 3);
    ASSERT(paging_get_physical(virtual_address) == physical_address);
    ASSERT(*(volatile uint32_t *)virtual_address == 0x0BADF00D);

    paging_unmap(virtual_address);
    ASSERT(paging_get_physical(virtual_address) == 0);
}

static const test_case_t cases[] = {
    {"map_and_unmap", test_map_and_unmap},
    {"hardware_translation", test_hardware_translation},
    {"map_beyond_first_4mb", test_map_beyond_first_4mb},
};

void test_suite_paging(void)
{
    test_run_suite("paging", cases, sizeof(cases) / sizeof(cases[0]));
}
