#ifndef GUI_H
#define GUI_H

#include "types.h"

#define GUI_MAX_WINDOWS 8

typedef struct gui_window gui_window_t;

// Called from gui_render() to paint the window's content area.
typedef void (*gui_draw_fn)(gui_window_t *window);
// Called with every keystroke while the window is focused.
typedef void (*gui_key_fn)(gui_window_t *window, char c);

struct gui_window
{
    const char *title;
    int32_t x, y;
    uint32_t width, height;
    uint32_t bg_color;
    gui_draw_fn on_draw;
    gui_key_fn on_key;
    void *user_data; 
};

int gui_initialize(uint32_t multiboot_info_address, uint32_t width, uint32_t height, uint8_t bpp);
int gui_is_active(void);

// Switches into graphics mode and renders the desktop.
void gui_enter(void);

// Registers a window with the desktop, front-to-back in the order added.
// Returns its index, or -1 if GUI_MAX_WINDOWS is already reached.
int gui_add_window(gui_window_t *window);

// Redraws the desktop background and every registered window. 
// Redraws the whole screen pixel by pixel, so prefer gui_redraw_window() when only one
// window's contents changed (e.g. from an on_key handler).
void gui_render(void);

// Redraws a single window's chrome and content, without touching the rest of
// the desktop. Much cheaper than gui_render() for per-keystroke updates.
void gui_redraw_window(gui_window_t *window);

// Moves keyboard focus to the next window (wraps around); the focused
// window's title bar is highlighted. Called automatically on Tab.
void gui_focus_next(void);

// Routes a keystroke to the focused window, except Tab which switches focus.
void gui_handle_key(char c);

// Content-area geometry helpers, i.e. the area below a window's title bar,
// for use inside a window's on_draw/on_key callbacks.
uint32_t gui_window_content_x(const gui_window_t *window);
uint32_t gui_window_content_y(const gui_window_t *window);
uint32_t gui_window_content_width(const gui_window_t *window);
uint32_t gui_window_content_height(const gui_window_t *window);

#endif // GUI_H
