#include "terminal.h"
#include "interrupts.h"
#include "commands.h"
#include "pmm.h"
#include "paging.h"

unsigned int multiboot_info_address;

void kmain(unsigned int magic, unsigned int multiboot_info)
{
    (void)magic;

    multiboot_info_address = multiboot_info;

    shell_initialize();
    interrupts_initialize();
    pmm_initialize(multiboot_info_address);
    paging_initialize();

    while (1)
    {
        __asm__ volatile("hlt");
    }
}