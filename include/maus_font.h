#ifndef MAUS_FONT_H
#define MAUS_FONT_H

#include "uletypes.h"

#include "maus.h"

#define MAUS_BDF_GLYPHS_MAX 256

typedef struct {
	ule_u8* bmp;
	ule_u16 w;
	ule_u16 h;
	ule_i16 xoff;
	ule_i16 yoff;
	ule_u16 adv;
	ule_i8  valid;
} MausGlyph;

typedef struct {
	MausGlyph glyphs[MAUS_BDF_GLYPHS_MAX];

	ule_i32 asc;
	ule_i32 dsc;
	ule_i32 cellh;
	ule_i32 cellw;
} MausFont;

/* Loops through text passing parameters to glyph drawer */
void maus_draw_text(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    const char* text, MausColor col);

/* load a bdf font into MausFont. returns NULL on fail */
MausFont* maus_font_load(const char* path);

/* release all allocations made with a MausFont */
void maus_font_free(MausFont* font);

#endif /* MAUS_FONT_H */

