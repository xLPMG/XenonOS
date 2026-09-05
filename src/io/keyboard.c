#include "keyboard.h"
#include "string.h"
#include "terminal.h"
#include "commands.h"
#include "types.h"
#include "io_helper.h"
#include "gui.h"

#define INPUT_SIZE 128

static int shift = 0;
static int caps_lock = 0;

static char input[INPUT_SIZE];
static int input_length = 0;

static const char normal_table[128] = {
    0, 27,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',
    0,
    '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/',
    0,
    '*',
    0,
    ' '};

static const char shift_table[128] = {
    0, 27,
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b',
    '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}', '\n',
    0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"', '~',
    0,
    '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M',
    '<', '>', '?',
    0,
    '*',
    0,
    ' '};

void keyboard_handler(void)
{
    uint8_t scancode = inb(0x60);

    // Key release
    if (scancode & 0x80)
    {
        uint8_t key = scancode & 0x7F;

        if (key == 42 || key == 54)
            shift = 0;

        return;
    }

    // Shift
    if (scancode == 42 || scancode == 54)
    {
        shift = 1;
        return;
    }

    // Caps Lock
    if (scancode == 58)
    {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode >= 128)
        return;

    char c;

    if (shift)
        c = shift_table[scancode];
    else
        c = normal_table[scancode];

    // Caps Lock
    if (caps_lock && c >= 'a' && c <= 'z')
        c -= 'a' - 'A';

    if (caps_lock && !shift && c >= 'A' && c <= 'Z')
        c += 'a' - 'A';

    // While the GUI owns the screen, it owns keystrokes too - permanently,
    // since there's no safe way back to the text shell without a reboot.
    if (gui_is_active())
    {
        gui_handle_key(c);
        return;
    }

    // Backspace
    if (c == '\b')
    {
        if (input_length > 0)
        {
            input_length--;
            terminal_backspace();
        }
        input[input_length] = '\0';

        return;
    }

    // Enter
    if (c == '\n')
    {
        execute_command(input, input_length);
        input_length = 0;
        return;
    }

    // Normal character
    if (c && input_length < INPUT_SIZE - 1)
    {
        input[input_length++] = c;
        terminal_putchar(c);
    }
}