#ifndef MAUS_FONT_H
#define MAUS_FONT_H

#define ULE_SHORT_TYPE_NAMES
#include "uletypes.h"

#include "maus.h"

#define MAUS_BDF_GLYPHS_MAX 256

typedef struct {
	u8* bmp;
	u16 w;
	u16 h;
	i16 xoff;
	i16 yoff;
	u16 adv;
	i8  valid;
} MausGlyph;

typedef struct {
	MausGlyph glyphs[MAUS_BDF_GLYPHS_MAX];

	i32 asc;
	i32 dsc;
	i32 cellh;
	i32 cellw;
} MausFont;

/* Loops through text passing parameters to glyph drawer */
void maus_draw_text(Maus* mw, MausFont* font, i32 x, i32 y,
                    const char* text, MausColor col);

/* load a bdf font into MausFont. returns NULL on fail */
MausFont* maus_font_load(const char* path);

/* release all allocations made with a MausFont */
void maus_font_free(MausFont* font);

#endif /* MAUS_FONT_H */

