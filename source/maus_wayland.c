#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include "maus.h"
#include "maus_wayland.h"

static void registry_global(void* data, struct wl_registry* registry,
                            uint32_t name, const char* interface,
                            uint32_t version);
static void registry_global_remove(void *data, struct wl_registry *wl_registry,
                          uint32_t name);

static struct wl_registry_listener registry_listener = {
	registry_global, registry_global_remove
};

static void registry_global(void* data, struct wl_registry* registry,
                            uint32_t name, const char* interface,
                            uint32_t version)
{
	Maus* mw = data;
	MausBackend* be = &mw->backend;

	(void) version;

	if (strcmp(interface, wl_compositor_interface.name) == 0)
		be->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	else if (strcmp(interface, wl_shm_interface.name) == 0)
		be->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
		be->wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
}

static void registry_global_remove(void *data, struct wl_registry *wl_registry,
                          uint32_t name)
{

}

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

Maus* maus_init(const char* title, int x, int y, int width, int height)
{
	Maus* mw = calloc(1, sizeof(Maus));
	MausBackend* be = &mw->backend;

	if (!mw)
		return NULL;

	be->display = wl_display_connect(NULL);
	if (!be->display) {
		free(mw);
		return NULL;
	}

	be->registry = wl_display_get_registry(be->display);
	wl_registry_add_listener(be->registry, &registry_listener, mw);
	wl_display_roundtrip(be->display);

	if (!be->compositor || !be->shm || !be->wm_base) {
		maus_close(mw);
		free(mw);
		return NULL;
	}

	mw->frame_time_last = maus_get_time_ns();
	mw->title = title;
	mw->x = x;
	mw->y = y;
	mw->width = width;
	mw->height = height;
	mw->stride = width;

	mw->bfb = calloc(mw->stride * mw->height, sizeof(uint32_t));
	if (!mw->bfb) {
		maus_close(mw);
		free(mw);
		return NULL;
	}

	return mw;
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

void maus_cur_set_mode(Maus* mw, MausCursorState state)
{

}

