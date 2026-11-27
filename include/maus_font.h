#ifndef MAUS_FONT_H
#define MAUS_FONT_H

#include "uletypes.h"

#include "maus.h"

#define MAUS_BDF_GLYPHS_MAX 0x110000
#define MAUS_UNICODE_PLANES 17
#define MAUS_GLYPH_PAGE_SIZE 256

typedef struct {
	ule_u8* bmp;
	ule_u16 w;
	ule_u16 h;
	ule_i16 xoff;
	ule_i16 yoff;
	ule_u16 adv;
	ule_i8  valid;

	ule_u16 left;
	ule_u16 top;
	ule_u16 right;
	ule_u16 bottom;

} MausGlyph;

typedef struct {
	MausGlyph** glyphs[MAUS_UNICODE_PLANES];

	ule_i32 asc;
	ule_i32 dsc;
	ule_i32 cellh;
	ule_i32 cellw;
} MausFont;

/* draw a codepoint to screen buffer (unicode supported) */
void maus_draw_char(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    ule_i32 codepoint, MausColor col);

/* draw a string to screen buffer (unicode supported) */
void maus_draw_text(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    const char* text, MausColor col);

/* get texts bounds relative to drawing origin. returns 0 values on fail */
void maus_get_text_bounds(MausFont* font, const char* text,
                          ule_i32* x, ule_i32* y, ule_i32* width, ule_i32* height);

/* get width of text in pixels */
ule_i32 maus_get_text_width(MausFont* font, const char* text);

/* get height of text in pixels */
ule_i32 maus_get_text_height(MausFont* font, const char* text);

/* load a bdf font into MausFont. returns NULL on fail */
MausFont* maus_font_load(const char* path);

/* release all allocations made with a MausFont */
void maus_font_free(MausFont* font);

#endif /* MAUS_FONT_H */

