#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "maus.h"
#include "maus_wayland.h"

static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);
static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial);
static void xdg_toplevel_configure(void* data, struct xdg_toplevel* xdg_toplevel,
                                   int32_t width, int32_t height, struct wl_array* states);
static void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel);
static void xdg_toplevel_configure_bounds(void* data, struct xdg_toplevel* xdg_toplevel,
                                          int32_t width, int32_t height);
static void xdg_toplevel_wm_capabilities(void* data, struct xdg_toplevel* xdg_toplevel,
                                         struct wl_array* capabilities);
static void wm_base_ping(void* data, struct xdg_wm_base* wm_base, uint32_t serial);
static void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
static void seat_name(void* data, struct wl_seat* seat, const char* name);
static void pointer_enter(void* data, struct wl_pointer* pointer, uint32_t serial,
                          struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
static void pointer_leave(void* data, struct wl_pointer* pointer, uint32_t serial,
                          struct wl_surface* surface);
static void pointer_motion(void* data, struct wl_pointer* pointer, uint32_t time,
                           wl_fixed_t sx, wl_fixed_t sy);
static void pointer_button(void* data, struct wl_pointer* pointer, uint32_t serial,
                           uint32_t time, uint32_t button, uint32_t state);
static void pointer_axis(void* data, struct wl_pointer* pointer, uint32_t time,
                         uint32_t axis, wl_fixed_t value);
static void pointer_frame(void* data, struct wl_pointer* pointer);
static void pointer_axis_source(void* data, struct wl_pointer* pointer, uint32_t axis_source);
static void pointer_axis_stop(void* data, struct wl_pointer* pointer, uint32_t time,
                              uint32_t axis);
static void pointer_axis_discrete(void* data, struct wl_pointer* pointer, uint32_t axis,
                                  int32_t discrete);
static void pointer_axis_value120(void* data, struct wl_pointer* pointer, uint32_t axis,
                                  int32_t value120);
static void pointer_axis_relative_direction(void* data, struct wl_pointer* pointer,
                                            uint32_t axis, uint32_t direction);
static void keyboard_keymap(void* data, struct wl_keyboard* keyboard, uint32_t format,
                            int32_t fd, uint32_t size);
static void keyboard_enter(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                           struct wl_surface* surface, struct wl_array* keys);
static void keyboard_leave(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                           struct wl_surface* surface);
static void keyboard_key(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                         uint32_t time, uint32_t key, uint32_t state);
static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched,
                               uint32_t mods_locked, uint32_t group);
static void keyboard_repeat_info(void* data, struct wl_keyboard* keyboard,
                                 int32_t rate, int32_t delay);
static void registry_global(void* data, struct wl_registry* registry,
                            uint32_t name, const char* interface,
                            uint32_t version);
static MausKey keysym_to_mauskey(xkb_keysym_t sym);
static MausMouseButton wl_button_to_maus(uint32_t button);
static struct wl_registry_listener registry_listener = { registry_global, registry_global_remove };
static struct xdg_surface_listener xdg_surface_listener = { xdg_surface_configure };
static struct xdg_toplevel_listener xdg_toplevel_listener = {
	xdg_toplevel_configure, xdg_toplevel_close,
	xdg_toplevel_configure_bounds, xdg_toplevel_wm_capabilities
};
static struct xdg_wm_base_listener xdg_wm_base_listener = { wm_base_ping };
static struct wl_seat_listener seat_listener = { seat_capabilities, seat_name };
static struct wl_pointer_listener pointer_listener = {
	pointer_enter, pointer_leave, pointer_motion, pointer_button,
	pointer_axis, pointer_frame, pointer_axis_source, pointer_axis_stop,
	pointer_axis_discrete, pointer_axis_value120, pointer_axis_relative_direction
};
static struct wl_keyboard_listener keyboard_listener = {
	keyboard_keymap, keyboard_enter, keyboard_leave, keyboard_key,
	keyboard_modifiers, keyboard_repeat_info
};

static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) { }

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
	Maus* mw = data;
	xdg_surface_ack_configure(xdg_surface, serial);
	mw->backend.configured = 1;
	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_REDRAW;
}

static void xdg_toplevel_configure(void* data, struct xdg_toplevel* xdg_toplevel,
                                   int32_t width, int32_t height, struct wl_array* states)
{
	Maus* mw = data;

	(void)xdg_toplevel;
	(void)states;

	if (width <= 0 || height <= 0)
		return;

	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_RESIZE;
	mw->backend.pending.width = width;
	mw->backend.pending.height = height;
}

static void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel)
{
	Maus* mw = data;

	(void)xdg_toplevel;

	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_CLOSE;
}

static void xdg_toplevel_configure_bounds(void* data, struct xdg_toplevel* xdg_toplevel,
                                          int32_t width, int32_t height) { }

static void xdg_toplevel_wm_capabilities(void* data, struct xdg_toplevel* xdg_toplevel,
                                         struct wl_array* capabilities) { }

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
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		be->wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(be->wm_base, &xdg_wm_base_listener, mw);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0) {
		be->seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
		wl_seat_add_listener(be->seat, &seat_listener, mw);
	}
}

static void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities)
{
	Maus* mw = data;
	MausBackend* be = &mw->backend;

	if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !be->pointer) {
		be->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(be->pointer, &pointer_listener, mw);
	}
	else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && be->pointer) {
		wl_pointer_destroy(be->pointer);
		be->pointer = NULL;
	}

	if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !be->keyboard) {
		be->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(be->keyboard, &keyboard_listener, mw);
	}
	else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && be->keyboard) {
		wl_keyboard_destroy(be->keyboard);
		be->keyboard = NULL;
	}
}

static void seat_name(void* data, struct wl_seat* seat, const char* name) { }

static void pointer_enter(void* data, struct wl_pointer* pointer, uint32_t serial,
                          struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) { }

static void pointer_leave(void* data, struct wl_pointer* pointer, uint32_t serial,
                          struct wl_surface* surface) { }

static void pointer_motion(void* data, struct wl_pointer* pointer, uint32_t time,
                           wl_fixed_t sx, wl_fixed_t sy)
{
	Maus* mw = data;

	(void)pointer;
	(void)time;

	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_MOUSE_MOTION;
	mw->backend.pending.mouse_x = wl_fixed_to_int(sx);
	mw->backend.pending.mouse_y = wl_fixed_to_int(sy);
}

static void pointer_button(void* data, struct wl_pointer* pointer, uint32_t serial,
                           uint32_t time, uint32_t button, uint32_t state)
{
	Maus* mw = data;
	MausMouseButton mb;

	(void)pointer;
	(void)serial;
	(void)time;

	mb = wl_button_to_maus(button);
	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_MOUSE_BUTTON;
	mw->backend.pending.mouse_button = mb;
	mw->backend.pending.mouse_pressed = state == WL_POINTER_BUTTON_STATE_PRESSED;

	if (mb != MAUS_MOUSE_BUTTON_NONE)
		mw->mouse_buttons[mb] = mw->backend.pending.mouse_pressed;
}

static void pointer_axis(void* data, struct wl_pointer* pointer, uint32_t time,
                         uint32_t axis, wl_fixed_t value)
{
	Maus* mw = data;

	(void)pointer;
	(void)time;

	if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
		return;

	mw->backend.pending.pending = 1;
	mw->backend.pending.type = MAUS_EV_MOUSE_BUTTON;
	mw->backend.pending.mouse_button = wl_fixed_to_int(value) > 0
		? MAUS_MOUSE_BUTTON_SCROLL_DOWN
		: MAUS_MOUSE_BUTTON_SCROLL_UP;
	mw->backend.pending.mouse_pressed = 1;
}

static void pointer_frame(void* data, struct wl_pointer* pointer) { }

static void pointer_axis_source(void* data, struct wl_pointer* pointer, uint32_t axis_source) { }

static void pointer_axis_stop(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis) {
}

static void pointer_axis_discrete(void* data, struct wl_pointer* pointer, uint32_t axis,
                                  int32_t discrete) { }

static void pointer_axis_value120(void* data, struct wl_pointer* pointer, uint32_t axis,
                                  int32_t value120) { }

static void pointer_axis_relative_direction(void* data, struct wl_pointer* pointer,
                                            uint32_t axis, uint32_t direction) { }

static void keyboard_keymap(void* data, struct wl_keyboard* keyboard, uint32_t format,
                            int32_t fd, uint32_t size)
{
	Maus* mw = data;
	MausBackend* be = &mw->backend;
	char* map;

	(void)keyboard;

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		return;
	}

	if (be->xkb_state)
		xkb_state_unref(be->xkb_state);
	if (be->xkb_keymap)
		xkb_keymap_unref(be->xkb_keymap);

	be->xkb_keymap = xkb_keymap_new_from_string(
		be->xkb_context, map,
		XKB_KEYMAP_FORMAT_TEXT_V1,
		XKB_KEYMAP_COMPILE_NO_FLAGS
	);

	munmap(map, size);
	close(fd);

	if (!be->xkb_keymap)
		return;

	be->xkb_state = xkb_state_new(be->xkb_keymap);
}

static void keyboard_enter(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                           struct wl_surface* surface, struct wl_array* keys) { }

static void keyboard_leave(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                           struct wl_surface* surface) { }

static void keyboard_key(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                         uint32_t time, uint32_t key, uint32_t state)
{
	Maus* mw = data;
	MausBackend* be = &mw->backend;
	uint32_t code;
	xkb_keysym_t sym;
	char text[8];
	int len;

	(void)keyboard;
	(void)serial;
	(void)time;

	code = key + 8;
	sym = XKB_KEY_NoSymbol;
	text[0] = 0;

	if (be->xkb_state) {
		sym = xkb_state_key_get_one_sym(be->xkb_state, code);
		if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			len = xkb_state_key_get_utf8(be->xkb_state, code, text, sizeof(text));
			if (len <= 0)
				text[0] = 0;
		}
	}

	be->pending.pending = 1;
	be->pending.type = MAUS_EV_KEY;
	be->pending.key_code = code;
	be->pending.key_sym = keysym_to_mauskey(sym);
	be->pending.key_text = text[0];
	be->pending.key_pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;

	if (code < MAUS_KEYCODE_LAST)
		mw->key_codes[code] = be->pending.key_pressed;
	if (be->pending.key_sym != MAUS_KEY_NONE)
		mw->key_syms[be->pending.key_sym] = be->pending.key_pressed;
}

static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched,
                               uint32_t mods_locked, uint32_t group)
{
	Maus* mw = data;

	(void)keyboard;
	(void)serial;

	if (!mw->backend.xkb_state)
		return;

	xkb_state_update_mask(
		mw->backend.xkb_state, mods_depressed,
		mods_latched, mods_locked,
		0, 0, group
	);
}

static void keyboard_repeat_info(void* data, struct wl_keyboard* keyboard,
                                 int32_t rate, int32_t delay) { }


static int8_t fb_create(Maus* mw);
static int8_t fb_create_shm(size_t size);
static int8_t handle_event(Maus* mw, MausEvent* ev);

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

static int8_t handle_event(Maus* mw, MausEvent* ev)
{
	MausBackend* be = &mw->backend;

	if (!be->pending.pending)
		return 0;

	ev->type = be->pending.type;
	be->pending.pending = 0;

	switch (ev->type) {
	case MAUS_EV_CLOSE:
		return 1;

	case MAUS_EV_RESIZE:
		ev->resize.width = be->pending.width;
		ev->resize.height = be->pending.height;
		return 1;

	case MAUS_EV_MOUSE_MOTION:
		ev->mouse.motion.x = be->pending.mouse_x;
		ev->mouse.motion.y = be->pending.mouse_y;
		return 1;

	case MAUS_EV_MOUSE_BUTTON:
		ev->mouse.button.button = be->pending.mouse_button;
		ev->mouse.button.pressed = be->pending.mouse_pressed;
		return 1;

	case MAUS_EV_KEY:
		ev->key.code = be->pending.key_code;
		ev->key.pressed = be->pending.key_pressed;
		ev->key.key = be->pending.key_sym;
		ev->key.text = be->pending.key_text;
		return 1;

	default:
		return 1;
	}
}

static MausMouseButton wl_button_to_maus(uint32_t button)
{
	switch (button) {
		case 0x110: return MAUS_MOUSE_BUTTON_LEFT;
		case 0x111: return MAUS_MOUSE_BUTTON_RIGHT;
		case 0x112: return MAUS_MOUSE_BUTTON_MIDDLE;
		default:    return MAUS_MOUSE_BUTTON_NONE;
	}
}

static MausKey keysym_to_mauskey(xkb_keysym_t sym)
{
	if (sym >= 0x20 && sym <= 0x7E)
		return (MausKey)sym;

	if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F12)
		return MAUS_KEY_F1 + (sym - XKB_KEY_F1);
	if (sym >= XKB_KEY_KP_0 && sym <= XKB_KEY_KP_9)
		return MAUS_KEY_KP_0 + (sym - XKB_KEY_KP_0);

	switch (sym) {
		case XKB_KEY_BackSpace:    return MAUS_KEY_BACKSPACE;
		case XKB_KEY_Tab:          return MAUS_KEY_TAB;
		case XKB_KEY_Return:       return MAUS_KEY_ENTER;
		case XKB_KEY_Escape:       return MAUS_KEY_ESCAPE;
		case XKB_KEY_Delete:       return MAUS_KEY_DELETE;
		case XKB_KEY_Left:         return MAUS_KEY_LEFT;
		case XKB_KEY_Right:        return MAUS_KEY_RIGHT;
		case XKB_KEY_Up:           return MAUS_KEY_UP;
		case XKB_KEY_Down:         return MAUS_KEY_DOWN;
		case XKB_KEY_Home:         return MAUS_KEY_HOME;
		case XKB_KEY_End:          return MAUS_KEY_END;
		case XKB_KEY_Page_Up:      return MAUS_KEY_PAGE_UP;
		case XKB_KEY_Page_Down:    return MAUS_KEY_PAGE_DOWN;
		case XKB_KEY_Insert:       return MAUS_KEY_INSERT;
		case XKB_KEY_Shift_L:      return MAUS_KEY_SHIFT_L;
		case XKB_KEY_Shift_R:      return MAUS_KEY_SHIFT_R;
		case XKB_KEY_Control_L:    return MAUS_KEY_CONTROL_L;
		case XKB_KEY_Control_R:    return MAUS_KEY_CONTROL_R;
		case XKB_KEY_Alt_L:        return MAUS_KEY_ALT_L;
		case XKB_KEY_Alt_R:        return MAUS_KEY_ALT_R;
		case XKB_KEY_Super_L:      return MAUS_KEY_SUPER_L;
		case XKB_KEY_Super_R:      return MAUS_KEY_SUPER_R;
		case XKB_KEY_Caps_Lock:    return MAUS_KEY_CAPS_LOCK;
		case XKB_KEY_Num_Lock:     return MAUS_KEY_NUM_LOCK;
		case XKB_KEY_Scroll_Lock:  return MAUS_KEY_SCROLL_LOCK;
		case XKB_KEY_Print:        return MAUS_KEY_PRINT_SCREEN;
		case XKB_KEY_Pause:        return MAUS_KEY_PAUSE;
		case XKB_KEY_Menu:         return MAUS_KEY_MENU;
		case XKB_KEY_KP_Decimal:   return MAUS_KEY_KP_DECIMAL;
		case XKB_KEY_KP_Divide:    return MAUS_KEY_KP_DIVIDE;
		case XKB_KEY_KP_Multiply:  return MAUS_KEY_KP_MULTIPLY;
		case XKB_KEY_KP_Subtract:  return MAUS_KEY_KP_SUBTRACT;
		case XKB_KEY_KP_Add:       return MAUS_KEY_KP_ADD;
		case XKB_KEY_KP_Enter:     return MAUS_KEY_KP_ENTER;
		case XKB_KEY_KP_Equal:     return MAUS_KEY_KP_EQUAL;
		default:                   return MAUS_KEY_NONE;
	}
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

	if (be->keyboard)
		wl_keyboard_destroy(be->keyboard);

	if (be->pointer)
		wl_pointer_destroy(be->pointer);

	if (be->seat)
		wl_seat_destroy(be->seat);

	if (be->xkb_state)
		xkb_state_unref(be->xkb_state);

	if (be->xkb_keymap)
		xkb_keymap_unref(be->xkb_keymap);

	if (be->xkb_context)
		xkb_context_unref(be->xkb_context);

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

	xdg_toplevel_add_listener(be->xdg_toplevel, &xdg_toplevel_listener, mw);
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

	be->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!be->xkb_context) {
		maus_close(mw);
		free(mw);
		return NULL;
	}

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
	MausBackend* be = &mw->backend;
	struct pollfd pfd;

	ev->type = MAUS_EV_NONE;

	if (handle_event(mw, ev))
		return 1;

	wl_display_dispatch_pending(be->display);
	if (handle_event(mw, ev))
		return 1;

	if (wl_display_prepare_read(be->display) != 0) {
		wl_display_dispatch_pending(be->display);
		return handle_event(mw, ev);
	}

	wl_display_flush(be->display);

	pfd.fd = wl_display_get_fd(be->display);
	pfd.events = POLLIN;
	pfd.revents = 0;
	
	if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
		if (wl_display_read_events(be->display) == -1) {
			ev->type = MAUS_EV_CLOSE;
			wl_display_cancel_read(be->display);
			return 1;
		}
	}
	else {
		wl_display_cancel_read(be->display);
		return 0;
	}

	wl_display_dispatch_pending(be->display);

	return handle_event(mw, ev);
}

void maus_event_wait(Maus* mw, MausEvent* ev)
{
	MausBackend* be = &mw->backend;

	ev->type = MAUS_EV_NONE;

	for (;;) {
		if (handle_event(mw, ev))
			return;

		if (wl_display_dispatch(be->display) == -1) {
			ev->type = MAUS_EV_CLOSE;
			return;
		}
	}
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

