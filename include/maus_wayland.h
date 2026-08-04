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
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct wl_cursor_theme;
struct wl_cursor;

struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;

struct zwp_pointer_constraints_v1;
struct zwp_confined_pointer_v1;

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

typedef struct {
	int8_t  pending;
	int32_t type;

	uint32_t width;
	uint32_t height;

	uint32_t mouse_x;
	uint32_t mouse_y;
	uint32_t mouse_button;
	uint8_t  mouse_pressed;

	uint32_t key_code;
	uint32_t key_sym;
	char     key_text;
	uint8_t  key_pressed;
} MausEventPending;

typedef struct {
	struct wl_display*    display;
	struct wl_buffer*     buffer;
	struct wl_compositor* compositor;
	struct wl_surface*    surface;

	struct wl_shm* shm;
	void*          shm_data;
	size_t         shm_size;

	struct wl_seat*     seat;
	struct wl_pointer*  pointer;
	struct wl_keyboard* keyboard;

	struct wl_cursor_theme* cursor_theme;
	struct wl_cursor*       cursor;
	struct wl_surface*      cursor_surface;
	uint32_t                pointer_enter_serial;
	int8_t                  cursor_state;

	struct zwp_pointer_constraints_v1* pointer_constraints;
	struct zwp_confined_pointer_v1*    locked_pointer;

	struct xkb_context* xkb_context;
	struct xkb_keymap*  xkb_keymap;
	struct xkb_state*   xkb_state;


	MausEventPending pending;

	struct xdg_wm_base*  wm_base;
	struct xdg_surface*  xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	int8_t               configured;

	struct wl_registry*          registry;
} MausBackend;

#endif /* MAUS_WAYLAND_H */

