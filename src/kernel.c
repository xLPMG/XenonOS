#include "terminal.h"
#include "interrupts.h"
#include "commands.h"
#include "pmm.h"
#include "paging.h"
#include "tss.h"
#include "scheduler.h"
#include "pit.h"
#include "thread.h"
#include "constants.h"

unsigned int multiboot_info_address;

void kmain(unsigned int magic, unsigned int multiboot_info)
{
    (void)magic;

    multiboot_info_address = multiboot_info;

    shell_initialize();
    interrupts_initialize();
    pmm_initialize(multiboot_info_address);
    paging_initialize();
    tss_initialize();
    scheduler_initialize();

    // Only start the timer once the scheduler is fully ready
    pit_initialize(TIMER_HZ);

    while (1)
    {
        __asm__ volatile("hlt");
    }
}