#include "terminal.h"
#include "interrupts.h"
#include "commands.h"

unsigned int multiboot_info_address;

void kmain(unsigned int magic, unsigned int multiboot_info)
{
    (void)magic;

    multiboot_info_address = multiboot_info;

    clear();
    interrupts_initialize();

    while (1)
    {
        __asm__ volatile("hlt");
    }
}