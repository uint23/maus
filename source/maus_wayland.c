#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include "maus.h"
#include "maus_wayland.h"

static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);
static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial);
static void wm_base_ping(void* data, struct xdg_wm_base* wm_base, uint32_t serial);
static void registry_global(void* data, struct wl_registry* registry,
                            uint32_t name, const char* interface,
                            uint32_t version);
static struct wl_registry_listener registry_listener = { registry_global, registry_global_remove };
static struct xdg_surface_listener xdg_surface_listener = { xdg_surface_configure };
static struct xdg_wm_base_listener xdg_wm_base_listener = { wm_base_ping };

static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{

}

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
	Maus* mw = data;
	xdg_surface_ack_configure(xdg_surface, serial);
	mw->backend.configured = 1;
}

static void wm_base_ping(void* data, struct xdg_wm_base* wm_base, uint32_t serial)
{
	(void)data;
	xdg_wm_base_pong(wm_base, serial);
}

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
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		be->wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(be->wm_base, &xdg_wm_base_listener, mw);
	}
}




static int8_t fb_create(Maus* mw);
static int8_t fb_create_shm(size_t size);

static int8_t fb_create(Maus* mw)
{
	MausBackend* be = &mw->backend;
	struct wl_shm_pool* pool;

	int fd;
	int stride;
	int size;

	stride = mw->width * 4;
	size = stride * mw->height;

	fd = fb_create_shm(size);
	if (fd < 0)
		return 0;

	be->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (be->shm_data == MAP_FAILED) {
		close(fd);
		return 0;
	}

	pool = wl_shm_create_pool(be->shm, fd, size);
	be->buffer = wl_shm_pool_create_buffer(
		pool, 0, mw->width, mw->height,
		stride, WL_SHM_FORMAT_XRGB8888
	);

	wl_shm_pool_destroy(pool);
	close(fd);

	be->shm_size = size;
	mw->fb = be->shm_data;
	mw->stride = mw->width;

	return 1;
}

static int8_t fb_create_shm(size_t size)
{
	char name[64];
	int fd;

	int i;

	for (i = 0; i < 100; i++) {
		snprintf(name, sizeof(name), "/maus-%ld-%d", (long)getpid(), i);

		fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);

			if (ftruncate(fd, size) < 0) {
				close(fd);
				return -1;
			}

			return fd;
		}
	}

	return -1;
}

void maus_clear(Maus* mw, MausColor col)
{
	uint32_t up = MAUS_UNPACK_COL(col);
	uint32_t pxs = mw->height * mw->width;

	uint32_t i;

	for (i = 0; i < pxs; i++)
		mw->bfb[i] = up;
}

void maus_clipboard_set_text(Maus* mw, const char* text)
{

}

char* maus_clipboard_get_text(Maus* mw)
{

}

void maus_close(Maus* mw)
{
	MausBackend* be;

	if (!mw)
		return;

	be = &mw->backend;

	if (be->buffer)
		wl_buffer_destroy(be->buffer);

	if (be->shm_data && be->shm_size > 0)
		munmap(be->shm_data, be->shm_size);

	if (be->xdg_toplevel)
		xdg_toplevel_destroy(be->xdg_toplevel);

	if (be->xdg_surface)
		xdg_surface_destroy(be->xdg_surface);

	if (be->surface)
		wl_surface_destroy(be->surface);

	if (be->wm_base)
		xdg_wm_base_destroy(be->wm_base);

	if (be->shm)
		wl_shm_destroy(be->shm);

	if (be->compositor)
		wl_compositor_destroy(be->compositor);

	if (be->registry)
		wl_registry_destroy(be->registry);

	if (be->display) {
		wl_display_flush(be->display);
		wl_display_disconnect(be->display);
	}

	free(mw->bfb);
	free(mw);
}

int8_t maus_close_window(Maus* mw)
{
	MausBackend* be = &mw->backend;

	if (be->buffer)
		wl_buffer_destroy(be->buffer);
	if (be->shm_data && be->shm_size > 0)
		munmap(be->shm_data, be->shm_size);

	if (be->xdg_toplevel)
		xdg_toplevel_destroy(be->xdg_toplevel);
	if (be->xdg_surface)
		xdg_surface_destroy(be->xdg_surface);
	if (be->surface)
		wl_surface_destroy(be->surface);

	be->buffer = NULL;
	be->shm_data = NULL;
	be->shm_size = 0;
	be->xdg_toplevel = NULL;
	be->xdg_surface = NULL;
	be->surface = NULL;
	be->configured = 0;

	return 1;
}

int8_t maus_create_window(Maus* mw)
{
	MausBackend* be = &mw->backend;

	be->surface = wl_compositor_create_surface(be->compositor);
	if (!be->surface)
		return 0;

	be->xdg_surface = xdg_wm_base_get_xdg_surface(be->wm_base, be->surface);
	if (!be->xdg_surface)
		return 0;

	xdg_surface_add_listener(be->xdg_surface, &xdg_surface_listener, mw);

	be->xdg_toplevel = xdg_surface_get_toplevel(be->xdg_surface);
	if (!be->xdg_toplevel)
		return 0;

	xdg_toplevel_set_title(be->xdg_toplevel, mw->title);

	if (!fb_create(mw))
		return 0;

	wl_surface_commit(be->surface);

	while (!be->configured) {
		if (wl_display_dispatch(be->display) == -1)
			return 0;
	}

	return 1;
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
	ev->type = MAUS_EV_NONE;
	wl_display_dispatch_pending(mw->backend.display);
	wl_display_flush(mw->backend.display);
	return 0;
}

void maus_event_wait(Maus* mw, MausEvent* ev)
{

}

void maus_present(Maus* mw)
{
	MausBackend* be = &mw->backend;
	size_t bytes;

	if (!be->buffer)
		return;

	bytes = mw->stride * mw->height * sizeof(uint32_t);
	memcpy(be->shm_data, mw->bfb, bytes);

	wl_surface_attach(be->surface, be->buffer, 0, 0);
	wl_surface_damage_buffer(be->surface, 0, 0, mw->width, mw->height);
	wl_surface_commit(be->surface);
	wl_display_flush(be->display);
}

int8_t maus_resize(Maus* mw, uint32_t width, uint32_t height)
{

}

void maus_cur_set_mode(Maus* mw, MausCursorState state)
{

}

