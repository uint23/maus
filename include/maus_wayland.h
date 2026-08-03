#ifndef MAUS_WAYLAND_H
#define MAUS_WAYLAND_H

/* must forward declare so that other
   ANSI abiding files arent affected */
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shm;
struct xdg_wm_base;

typedef struct {
	struct wl_display*    display;
	struct wl_compositor* compositor;
	struct wl_shm*        shm;
	struct xdg_wm_base*   wm_base;

	struct wl_registry*          registry;
} MausBackend;

#endif /* MAUS_WAYLAND_H */
