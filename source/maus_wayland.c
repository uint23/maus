#include "maus.h"
#include "maus_wayland.h"

void maus_clear(Maus* mw, MausColor col)
{

}

void maus_clipboard_set_text(Maus* mw, const char* text)
{

}

char* maus_clipboard_get_text(Maus* mw)
{

}

void maus_close(Maus* mw)
{

}

int8_t maus_close_window(Maus* mw)
{

}

int8_t maus_create_window(Maus* mw)
{

}

void maus_die(const char* fmt, ...)
{

}

uint64_t maus_get_time_ns(void)
{

}

Maus* maus_init(const char* title, int x, int y, int width, int height)
{

}

void maus_log(FILE* fd, const char* fmt, ...)
{

}

int8_t maus_event_poll(Maus* mw, MausEvent* ev)
{

}

void maus_event_wait(Maus* mw, MausEvent* ev)
{

}

void maus_present(Maus* mw)
{

}

int8_t maus_resize(Maus* mw, uint32_t width, uint32_t height)
{

}

void maus_target_fps(Maus* mw, uint32_t fps)
{

}

void maus_cur_set_mode(Maus* mw, MausCursorState state)
{

}

