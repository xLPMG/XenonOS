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
#include "serial.h"
#include "graphics.h"
#include "gui.h"

static void help(void);
static void info(void);
static void echo(char *input);
static void shutdown(void);
static void uptime(void);
static void run_tests(void);
static void gui(void);

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
    else if (strcmp(input, "gui") == 0)
    {
        gui();
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
    terminal_write("  gui   - switch to graphics mode\n");
    terminal_write("\n> ");
}

static void info(void)
{
    char vendor[13];

    unsigned int family;
    unsigned int model;

    extern unsigned int multiboot_info_address;

    unsigned int memory_kb =
        memory_get_kb(multiboot_info_address);

    cpu_get_vendor(vendor);
    cpu_get_version(&family, &model);

    terminal_writef("XenonOS v%s\n", XENON_OS_VERSION);
    terminal_writef("CPU vendor: %s\n", vendor);
    terminal_writef("CPU family: %u\n", family);
    terminal_writef("CPU model: %u\n", model);
    terminal_writef("Memory: %u MB\n", memory_kb / 1024);

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

    terminal_writef("Uptime: %uh %um %us\n> ", hours, minutes % 60, seconds % 60);
}

static gui_window_t welcome_window;

static void welcome_window_draw(gui_window_t *window)
{
    uint32_t x = gui_window_content_x(window);
    uint32_t y = gui_window_content_y(window);

    gfx_draw_string(x + 4, y + 4, "Hello from XenonOS ^_^", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 4, y + 16, "Reboot to get back to the shell.", 0x00FFFFFF, window->bg_color);
}

static gui_window_t info_window;

static void info_window_draw(gui_window_t *window)
{
    uint32_t x = gui_window_content_x(window);
    uint32_t y = gui_window_content_y(window);

    char vendor[13];
    unsigned int family, model;
    char number[12];

    extern unsigned int multiboot_info_address;
    unsigned int memory_mb = memory_get_kb(multiboot_info_address) / 1024;

    cpu_get_vendor(vendor);
    cpu_get_version(&family, &model);

    gfx_draw_string(x + 4, y + 4, "CPU vendor:", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 96, y + 4, vendor, 0x00FFFFFF, window->bg_color);

    itoa(family, number);
    gfx_draw_string(x + 4, y + 16, "CPU family:", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 96, y + 16, number, 0x00FFFFFF, window->bg_color);

    itoa(model, number);
    gfx_draw_string(x + 4, y + 28, "CPU model:", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 96, y + 28, number, 0x00FFFFFF, window->bg_color);

    itoa(memory_mb, number);
    gfx_draw_string(x + 4, y + 40, "Memory (MB):", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 96, y + 40, number, 0x00FFFFFF, window->bg_color);
}

#define NOTES_BUFFER_SIZE 64

// user_data for notes_window, showcasing per-window state passed through the
// on_draw/on_key callbacks instead of relying on globals.
typedef struct
{
    char text[NOTES_BUFFER_SIZE];
    int length;
} notes_state_t;

static gui_window_t notes_window;
static notes_state_t notes_state;

static void notes_window_draw(gui_window_t *window)
{
    uint32_t x = gui_window_content_x(window);
    uint32_t y = gui_window_content_y(window);
    notes_state_t *state = (notes_state_t *)window->user_data;

    gfx_fill_rect(x, y, gui_window_content_width(window), gui_window_content_height(window), window->bg_color);
    gfx_draw_string(x + 4, y + 4, "Focused window - type something:", 0x00FFFFFF, window->bg_color);
    gfx_draw_string(x + 4, y + 16, state->text, 0x00FFFFFF, window->bg_color);
}

static void notes_window_key(gui_window_t *window, char c)
{
    notes_state_t *state = (notes_state_t *)window->user_data;

    if (c == '\b')
    {
        if (state->length > 0)
            state->text[--state->length] = '\0';
    }
    else if (c >= ' ' && state->length < NOTES_BUFFER_SIZE - 1)
    {
        state->text[state->length++] = c;
        state->text[state->length] = '\0';
    }

    gui_redraw_window(window);
}

// Switches to graphics mode on demand, keeping the VGA text console as the
// default so it stays usable until the user opts into the GUI. There is no
// way back once entered - see gui_is_active() in gui.h.
static void gui(void)
{
    extern unsigned int multiboot_info_address;

    if (!gui_initialize(multiboot_info_address, GUI_WIDTH, GUI_HEIGHT, 32))
    {
        terminal_write_colored("I couldn't find a VGA PCI device. No graphics mode for you >:(.\n\n> ", VGA_COLOR_RED, VGA_COLOR_BLACK);
        return;
    }

    // First window added gets keyboard focus - see gui_add_window() in gui.c.
    notes_window = (gui_window_t){
        .title = "Notes",
        .x = 40,
        .y = 40,
        .width = 360,
        .height = 100,
        .bg_color = 0x00202020,
        .on_draw = notes_window_draw,
        .on_key = notes_window_key,
        .user_data = &notes_state,
    };

    welcome_window = (gui_window_t){
        .title = "Welcome",
        .x = 440,
        .y = 40,
        .width = 320,
        .height = 100,
        .bg_color = 0x00202020,
        .on_draw = welcome_window_draw,
        .on_key = 0,
    };

    info_window = (gui_window_t){
        .title = "System Info",
        .x = 40,
        .y = 180,
        .width = 320,
        .height = 120,
        .bg_color = 0x00202020,
        .on_draw = info_window_draw,
        .on_key = 0,
    };

    gui_add_window(&notes_window);
    gui_add_window(&welcome_window);
    gui_add_window(&info_window);
    gui_enter();
}

static void run_tests(void)
{
    test_run_all();
}