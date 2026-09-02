#include "terminal.h"
#include "interrupts.h"
#include "commands.h"
#include "pmm.h"

unsigned int multiboot_info_address;

void kmain(unsigned int magic, unsigned int multiboot_info)
{
    (void)magic;

    multiboot_info_address = multiboot_info;

    clear();
    interrupts_initialize();
    pmm_initialize(multiboot_info_address);

    while (1)
    {
        __asm__ volatile("hlt");
    }
}