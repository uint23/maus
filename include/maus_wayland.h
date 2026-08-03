#ifndef MAUS_WAYLAND_H
#define MAUS_WAYLAND_H

#include <stddef.h>
#include <stdint.h>

/* must forward declare so that other
   ANSI abiding files arent affected */
struct wl_display;
struct wl_buffer;
struct wl_registry;
struct wl_compositor;
struct wl_surface;
struct wl_shm;

struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;

typedef struct {
	struct wl_display*    display;
	struct wl_buffer*     buffer;
	struct wl_compositor* compositor;
	struct wl_surface*    surface;
	struct wl_shm*        shm;
	void*                 shm_data;
	size_t                shm_size;

	struct xdg_wm_base*  wm_base;
	struct xdg_surface*  xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	int8_t               configured;

	struct wl_registry*          registry;
} MausBackend;

#endif /* MAUS_WAYLAND_H */
