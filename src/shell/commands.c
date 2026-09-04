#include "cpu.h"
#include "memory.h"
#include "commands.h"
#include "constants.h"
#include "terminal.h"
#include "string.h"
#include "pmm.h"
#include "paging.h"
#include "test.h"
#include "pit.h"

static void help(void);
static void info(void);
static void echo(char *input);
static void shutdown(void);
static void uptime(void);
static void run_tests(void);

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
    else if (strcmp(input, "uptime") == 0)
    {
        uptime();
    }
    else if (strcmp(input, "test") == 0)
    {
        run_tests();
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
    terminal_write("  uptime - show system uptime\n");
    terminal_write("  test  - run the unit test suite\n");
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

static void uptime(void)
{
    uint32_t ms = pit_get_ticks() * 1000 / TIMER_HZ;
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    char number[16];
    terminal_write("Uptime: ");
    itoa(hours, number);
    terminal_write(number);
    terminal_write("h ");
    itoa(minutes % 60, number);
    terminal_write(number);
    terminal_write("m ");
    itoa(seconds % 60, number);
    terminal_write(number);
    terminal_write("s\n> ");
}

static void run_tests(void)
{
    test_run_all();
}