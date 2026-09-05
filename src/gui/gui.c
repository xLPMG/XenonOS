#include "gui.h"
#include "framebuffer.h"
#include "graphics.h"
#include "serial.h"

#define TITLE_BAR_HEIGHT 18
#define BORDER_COLOR 0x00FFFFFFu
#define TITLE_BG_COLOR 0x00404040u
#define FOCUSED_TITLE_BG_COLOR 0x00355C8Cu
#define TITLE_FG_COLOR 0x00FFFFFFu
#define DESKTOP_BG_COLOR 0x00202020u

static gui_window_t *windows[GUI_MAX_WINDOWS];
static int window_count = 0;
static int focused_window = -1; // index into windows[], -1 = none
static int gui_active = 0;
static int gui_ready = 0; // framebuffer obtained via gui_initialize()

int gui_initialize(uint32_t multiboot_info_address, uint32_t width, uint32_t height, uint8_t bpp)
{
    if (!framebuffer_initialize(multiboot_info_address, width, height, bpp))
        return 0;

    gui_ready = 1;
    return 1;
}

int gui_is_active(void)
{
    return gui_active;
}

static void draw_window(const gui_window_t *window, int focused)
{
    uint32_t title_bg = focused ? FOCUSED_TITLE_BG_COLOR : TITLE_BG_COLOR;

    gfx_fill_rect(window->x, window->y, window->width, window->height, window->bg_color);
    gfx_draw_rect(window->x, window->y, window->width, window->height, BORDER_COLOR);

    gfx_fill_rect(window->x + 1, window->y + 1, window->width - 2, TITLE_BAR_HEIGHT, title_bg);
    if (window->title)
        gfx_draw_string(window->x + 6, window->y + 5, window->title, TITLE_FG_COLOR, title_bg);
}

void gui_render(void)
{
    if (!gui_ready)
        return;

    const struct framebuffer_info *fb = framebuffer_get_info();
    gfx_fill_rect(0, 0, fb->width, fb->height, DESKTOP_BG_COLOR);

    for (int i = 0; i < window_count; i++)
    {
        gui_window_t *window = windows[i];
        draw_window(window, i == focused_window);

        if (window->on_draw)
            window->on_draw(window);
    }

    framebuffer_present();
}

void gui_redraw_window(gui_window_t *window)
{
    if (!gui_ready)
        return;

    draw_window(window, focused_window >= 0 && windows[focused_window] == window);

    if (window->on_draw)
        window->on_draw(window);

    framebuffer_present();
}

int gui_add_window(gui_window_t *window)
{
    if (window_count >= GUI_MAX_WINDOWS)
        return -1;

    windows[window_count] = window;

    // First window added gets keyboard focus by default.
    if (focused_window == -1)
        focused_window = window_count;

    return window_count++;
}

void gui_enter(void)
{
    if (!gui_ready)
        return;

    gui_active = 1;
    gui_render();
    serial_write("gui: entered graphics mode\n");
}

void gui_focus_next(void)
{
    if (window_count <= 1)
        return;

    int previous = focused_window;
    focused_window = (focused_window + 1) % window_count;

    gui_redraw_window(windows[previous]);
    gui_redraw_window(windows[focused_window]);
}

void gui_handle_key(char c)
{
    if (!gui_active)
        return;

    // Tab cycles keyboard focus between windows instead of being forwarded.
    if (c == '\t')
    {
        gui_focus_next();
        return;
    }

    if (focused_window >= 0 && windows[focused_window]->on_key)
        windows[focused_window]->on_key(windows[focused_window], c);
}

uint32_t gui_window_content_x(const gui_window_t *window)
{
    return window->x + 1;
}

uint32_t gui_window_content_y(const gui_window_t *window)
{
    return window->y + TITLE_BAR_HEIGHT + 2;
}

uint32_t gui_window_content_width(const gui_window_t *window)
{
    return window->width - 2;
}

uint32_t gui_window_content_height(const gui_window_t *window)
{
    return window->height - TITLE_BAR_HEIGHT - 3;
}
