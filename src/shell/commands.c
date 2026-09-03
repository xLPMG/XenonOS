#include "cpu.h"
#include "memory.h"
#include "commands.h"
#include "terminal.h"
#include "string.h"
#include "pmm.h"
#include "paging.h"

static void help(void);
static void info(void);
static void echo(char *input);
static void shutdown(void);
static void memtest(void);

void execute_command(char *input, int input_length)
{
    terminal_putchar('\n');

    input[input_length] = '\0';

    if (strcmp(input, "") == 0)
    {
        terminal_write("> ");
    }
    else if (strcmp(input, "help") == 0)
    {
        help();
    }
    else if (strcmp(input, "clear") == 0)
    {
        shell_initialize();
    }
    else if (strcmp(input, "info") == 0)
    {
        info();
    }
    else if (strcmp(input, "shutdown") == 0 ||
             strcmp(input, "exit") == 0)
    {
        shutdown();
    }
    else if (strlen(input) >= 5 &&
             input[0] == 'e' &&
             input[1] == 'c' &&
             input[2] == 'h' &&
             input[3] == 'o' &&
             input[4] == ' ')
    {
        echo(input);
    }
    else if (strcmp(input, "memtest") == 0)
    {
        memtest();
    }
    else
    {
        terminal_write("Never heard of that. Try 'help' if you're unsure.\n\n>");
    }

    input_length = 0;
}

void shell_initialize(void)
{
    terminal_initialize();
    terminal_write("Hello from XenonOS ^_^\n\n> ");
}

static void help(void)
{
    terminal_write("Commands:\n");
    terminal_write("  help  - show this message\n");
    terminal_write("  clear - clear the screen\n");
    terminal_write("  echo  - print text\n");
    terminal_write("  info  - system information\n");
    terminal_write("  shutdown - shut down the system\n");
    terminal_write("\n> ");
}

static void info(void)
{
    char vendor[13];
    char number[16];

    unsigned int family;
    unsigned int model;

    extern unsigned int multiboot_info_address;

    unsigned int memory_kb =
        memory_get_kb(multiboot_info_address);

    cpu_get_vendor(vendor);
    cpu_get_version(&family, &model);

    terminal_write("XenonOS v");
    terminal_write(XENON_OS_VERSION);
    terminal_write("\n");

    terminal_write("CPU vendor: ");
    terminal_write(vendor);
    terminal_write("\n");

    terminal_write("CPU family: ");
    itoa(family, number);
    terminal_write(number);
    terminal_write("\n");

    terminal_write("CPU model: ");
    itoa(model, number);
    terminal_write(number);
    terminal_write("\n");

    terminal_write("Memory: ");
    itoa(memory_kb / 1024, number);
    terminal_write(number);
    terminal_write(" MB\n");

    terminal_write("Mode: 32-bit protected mode\n");
    terminal_write("Interrupts: enabled\n");

    terminal_write("\n> ");
}

static void shutdown(void)
{
    terminal_write("Shutting down...\n");

    __asm__ volatile("cli");

    while (1)
        __asm__ volatile("hlt");
}

static void echo(char *input)
{
    terminal_write(input + 5);
    terminal_write("\n> ");
}

// MARK: Debug Commands

static void memtest(void)
{
    uint32_t frame_0 = pmm_alloc();
    terminal_write("Allocated frame: ");
    char number[16];
    itoa_hex(frame_0, number);
    terminal_write(number);
    terminal_write("\n");

    uint32_t frame_1 = pmm_alloc();
    terminal_write("Allocated frame: ");
    itoa_hex(frame_1, number);
    terminal_write(number);
    terminal_write("\n");

    pmm_free(frame_1);
    terminal_write("Freed frame: ");
    itoa_hex(frame_1, number);
    terminal_write(number);
    terminal_write("\n");

    uint32_t frame_2 = pmm_alloc();
    terminal_write("Allocated frame: ");
    itoa_hex(frame_2, number);
    terminal_write(number);
    terminal_write("\n");

    pmm_free(frame_2);
    pmm_free(frame_0);

    // Paging test: remap a virtual page onto a different physical page and
    // verify the CPU sees the new translation (not a stale TLB entry).
    uint32_t test_virtual = 0x00200000;
    uint32_t test_physical = 0x00201000;

    *(volatile uint32_t *)test_physical = 0xDEADBEEF;

    paging_map(test_virtual, test_physical, 3);

    terminal_write("Mapped ");
    itoa_hex(test_virtual, number);
    terminal_write(number);
    terminal_write(" -> ");
    itoa_hex(paging_get_physical(test_virtual), number);
    terminal_write(number);
    terminal_write("\n");

    uint32_t read_back = *(volatile uint32_t *)test_virtual;
    terminal_write("Read back: ");
    itoa_hex(read_back, number);
    terminal_write(number);
    terminal_write(read_back == 0xDEADBEEF ? " (OK)\n" : " (FAIL)\n");

    paging_unmap(test_virtual);
    terminal_write("Unmapped, physical now: ");
    itoa_hex(paging_get_physical(test_virtual), number);
    terminal_write(number);
    terminal_write("\n");

    // Restore the identity mapping so the page stays usable afterwards.
    paging_map(test_virtual, test_virtual, 3);

    // Paging test: map a virtual address beyond the first 4MB, which needs a
    // page table to be allocated on demand for its directory entry.
    uint32_t high_virtual = 0x01000000;
    uint32_t high_physical = 0x00202000;

    *(volatile uint32_t *)high_physical = 0xCAFEF00D;

    paging_map(high_virtual, high_physical, 3);

    terminal_write("Mapped ");
    itoa_hex(high_virtual, number);
    terminal_write(number);
    terminal_write(" -> ");
    itoa_hex(paging_get_physical(high_virtual), number);
    terminal_write(number);
    terminal_write("\n");

    uint32_t high_read_back = *(volatile uint32_t *)high_virtual;
    terminal_write("Read back: ");
    itoa_hex(high_read_back, number);
    terminal_write(number);
    terminal_write(high_read_back == 0xCAFEF00D ? " (OK)\n" : " (FAIL)\n");

    paging_unmap(high_virtual);
    terminal_write("Unmapped, physical now: ");
    itoa_hex(paging_get_physical(high_virtual), number);
    terminal_write(number);
    terminal_write("\n> ");
}