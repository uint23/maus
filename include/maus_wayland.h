#ifndef MAUS_WAYLAND_H
#define MAUS_WAYLAND_H

#include <stddef.h>
#define ULE_SHORT_TYPE_NAMES
#include "uletypes.h"

#define MAUS_EVQ_MAX 64

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
struct zwp_locked_pointer_v1;
struct zwp_relative_pointer_manager_v1;
struct zwp_relative_pointer_v1;

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

typedef struct {
	i8  pending;
	i32 type;

	u32 width;
	u32 height;

	i32  mouse_x;
	i32  mouse_y;
	i32  mouse_dx;
	i32  mouse_dy;
	u32 mouse_button;
	u8  mouse_pressed;

	u32 key_code;
	u32 key_sym;
	char     key_text;
	u8  key_pressed;
} MausEventPending;

typedef struct {
	struct wl_buffer* buffer;
	void*             data;
	size_t            size;
	i8            busy;
} MausWLBuffer;

typedef struct {
	struct wl_display*    display;
	struct wl_compositor* compositor;
	struct wl_surface*    surface;

	struct wl_shm* shm;
	MausWLBuffer   buffers[2];

	struct wl_seat*     seat;
	struct wl_pointer*  pointer;
	struct wl_keyboard* keyboard;

	struct wl_cursor_theme* cursor_theme;
	struct wl_cursor*       cursor;
	struct wl_surface*      cursor_surface;
	u32                pointer_enter_serial;
	i32                 mouse_x;
	i32                 mouse_y;
	i8                  mouse_pos_set;
	i8                  cursor_visible;
	i8                  cursor_confined;

	struct zwp_pointer_constraints_v1*      pointer_constraints;
	struct zwp_confined_pointer_v1*         confined_pointer;
	struct zwp_locked_pointer_v1*           locked_pointer;
	struct zwp_relative_pointer_manager_v1* relative_pointer_manager;
	struct zwp_relative_pointer_v1*         relative_pointer;

	struct xkb_context* xkb_context;
	struct xkb_keymap*  xkb_keymap;
	struct xkb_state*   xkb_state;


	MausEventPending pending;
	MausEventPending evq[MAUS_EVQ_MAX];
	u32 evq_head;
	u32 evq_tail;
	u32 evq_count;

	i32  repeat_rate;
	i32  repeat_delay;
	u32 repeat_key_code;
	u32 repeat_key_sym;
	char     repeat_key_text;
	u64 repeat_next_ns;

	struct xdg_wm_base*  wm_base;
	struct xdg_surface*  xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	i8               configured;

	struct wl_registry*          registry;
} MausBackend;

#endif /* MAUS_WAYLAND_H */

