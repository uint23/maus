#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maus.h"
#include "maus_font.h"
#include "uleutf.h"

static ule_i32 glyph_advance(MausFont* font, MausGlyph* g);
static MausGlyph* get_glyph(MausFont* font, ule_i32 codepoint, int create);
static ule_i8 parse_glyphs(FILE* f, MausFont* font, char* line, size_t line_size,
                           ule_i32* encoding, ule_i32* advance, ule_i32* bw, ule_i32* bh,
                           ule_i32* xoff, ule_i32* yoff, ule_i8* seen);
static ule_u64 strtou64(const char* str);

static ule_i32 glyph_advance(MausFont* font, MausGlyph* g)
{
	if (g && g->valid)
		return g->adv;

	return font->cellw > 0 ? font->cellw : 8;
}

static MausGlyph* get_glyph(MausFont* font, ule_i32 codepoint, int create)
{
	int plane = 0;
	int page = 0;
	int index = 0;

	if (codepoint < 0 || codepoint >= MAUS_BDF_GLYPHS_MAX)
		return NULL;

	plane = codepoint >> 16;
	page = (codepoint >> 8) & 0xff;
	index = codepoint & 0xff;

	if (!font->glyphs[plane] && create)
		font->glyphs[plane] = calloc(MAUS_GLYPH_PAGE_SIZE, sizeof(MausGlyph*));
	if (!font->glyphs[plane])
		return NULL;

	if (!font->glyphs[plane][page] && create)
		font->glyphs[plane][page] = calloc(MAUS_GLYPH_PAGE_SIZE, sizeof(MausGlyph));
	if (!font->glyphs[plane][page])
		return NULL;

	return &font->glyphs[plane][page][index];
}

static ule_i8 parse_glyphs(FILE* f, MausFont* font, char* line, size_t line_size,
                           ule_i32* encoding, ule_i32* advance, ule_i32* bw, ule_i32* bh,
                           ule_i32* xoff, ule_i32* yoff, ule_i8* seen)
{
	ule_i32 rowbits = 0;

	ule_i32 x = 0;
	ule_i32 y = 0;
	ule_u64 bits = 0;

	MausGlyph* g = NULL;

	while (fgets(line, line_size, f)) {
		if (sscanf(line, "ENCODING %d", encoding) == 1)
			continue;
		if (sscanf(line, "DWIDTH %d", advance) == 1)
			continue;
		if (sscanf(line, "BBX %d %d %d %d", bw, bh, xoff, yoff) == 4)
			continue;

		if (strncmp(line, "BITMAP", 6) == 0) {
			/* invalid character code
			   TODO: just skip it */
			if (*encoding < 0 || *encoding >= MAUS_BDF_GLYPHS_MAX)
				break;

			/* empty/invalid size */
			if (*bw <= 0 || *bh <= 0)
				break;

			g = get_glyph(font, *encoding, 1);
			if (!g) {
				fclose(f);
				maus_font_free(font);
				return 0;
			}
			free(g->bmp); /* prevent leak on hot reload */
			memset(g, 0, sizeof(*g));

			g->w = *bw;
			g->h = *bh;
			g->xoff = *xoff;
			g->yoff = *yoff;
			g->adv = *advance;
			g->left = g->w;
			g->top = g->h;

			g->bmp = calloc((size_t)(*bw) * (*bh), 1);
			if (!g->bmp) {
				fclose(f);
				maus_font_free(font);
				return 0;
			}

			/* each bmp row in BDF is padded as a multiple
			   of 8 bits. this rounds width up to the next
			   multiple of 8 */
			rowbits = (*bw + 7) & ~7;

			for (y = 0; y < *bh; y++) {
				/* missing row */
				if (!fgets(line, line_size, f)) {
					fclose(f);
					maus_font_free(font);
					return 0;
				}

				bits = strtou64(line);
				for (x = 0; x < *bw; x++) {
					int bit = rowbits - 1 - x;

					if ((bits >> bit) & 1) {
						g->bmp[y * (*bw) + x] = 255;
						if (x < g->left)
							g->left = x;
						if (y < g->top)
							g->top = y;
						if (x + 1 > g->right)
							g->right = x + 1;
						if (y + 1 > g->bottom)
							g->bottom = y + 1;
					}
				}
			}

			g->valid = 1;
			if (*advance > font->cellw)
				font->cellw = *advance;
			*seen = 1;

			continue;
		}

		if (strncmp(line, "ENDCHAR", 7) == 0)
			break;
	}

	return 1;
}

static ule_u64 strtou64(const char* str)
{
	ule_u64 n = 0;
	ule_u8 c = 0;

	while (*str) {
		c = (ule_u8)*str++;

		if (c >= '0' && c <= '9')
			c = (ule_u8)(c - '0');
		else if (c >= 'A' && c <= 'F')
			c = (ule_u8)(c - 'A' + 10);
		else if (c >= 'a' && c <= 'f')
			c = (ule_u8)(c - 'a' + 10);
		else
			break;

		n = (n << 4) | c;
	}

	return n;
}

void maus_draw_char(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    ule_i32 codepoint, MausColor col)
{
	ule_i32 gox = 0;
	ule_i32 goy = 0;
	ule_i32 gx = 0;
	ule_i32 gy = 0;
	MausGlyph* g = NULL;

	if (!mw || !mw->bfb || !font || !col.a)
		return;

	g = get_glyph(font, codepoint, 0);
	if (!g || !g->valid)
		return;

	gox = x + g->xoff;
	goy = y + font->asc - g->h - g->yoff;

	for (gy = 0; gy < g->h; gy++) {
		for (gx = 0; gx < g->w; gx++) {
			if (g->bmp[gy * g->w + gx])
				maus_set_pixel(mw, gox + gx, goy + gy, col);
		}
	}
}

void maus_draw_text(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    const char* text, MausColor col)
{
	ule_i32 cp = 0;

	/* current coordinates */
	ule_i32 cx = x;
	ule_i32 cy = y;

	if (!mw || !mw->bfb || !font || !text)
		return;

	while (*text) {
		text = uleutf_codepoint(text, &cp);

		if (cp == '\n') {
			cx = x;
			cy += font->cellh;
			continue;
		}

		maus_draw_char(mw, font, cx, cy, cp, col);

		cx += glyph_advance(font, get_glyph(font, cp, 0));
	}
}

void maus_get_text_bounds(MausFont* font, const char* text,
                          ule_i32* x, ule_i32* y, ule_i32* width, ule_i32* height)
{
	ule_i32 cp = 0;
	ule_i32 cx = 0;
	ule_i32 cy = 0;
	ule_i32 left = 0;
	ule_i32 top = 0;
	ule_i32 right = 0;
	ule_i32 bottom = 0;
	ule_i32 gx = 0;
	ule_i32 gy = 0;
	MausGlyph* g = NULL;
	int seen = 0;

	while (font && text && *text) {
		text = uleutf_codepoint(text, &cp);

		if (cp == '\n') {
			cx = 0;
			cy += font->cellh;
			continue;
		}

		g = get_glyph(font, cp, 0);
		if (g && g->valid && g->left < g->right) {
			gx = cx + g->xoff;
			gy = cy + font->asc - g->h - g->yoff;

			if (!seen || gx + g->left < left)
				left = gx + g->left;
			if (!seen || gy + g->top < top)
				top = gy + g->top;
			if (!seen || gx + g->right > right)
				right = gx + g->right;
			if (!seen || gy + g->bottom > bottom)
				bottom = gy + g->bottom;
			seen = 1;
		}

		cx += glyph_advance(font, g);
	}

	if (x)
		*x = left;
	if (y)
		*y = top;
	if (width)
		*width = right - left;
	if (height)
		*height = bottom - top;
}

ule_i32 maus_get_text_width(MausFont* font, const char* text)
{
	ule_i32 cp = 0;
	ule_i32 width = 0;
	ule_i32 max = 0;
	if (!font || !text)
		return 0;

	while (*text) {
		text = uleutf_codepoint(text, &cp);

		if (cp == '\n') {
			width = 0;
			continue;
		}

		width += glyph_advance(font, get_glyph(font, cp, 0));
		if (width > max)
			max = width;
	}

	return max;
}

ule_i32 maus_get_text_height(MausFont* font, const char* text)
{
	ule_i32 height = 0;

	if (!font || !text || !*text)
		return 0;

	height = font->cellh;
	while (*text) {
		if (*text++ == '\n')
			height += font->cellh;
	}

	return height;
}

MausFont* maus_font_load(const char* path)
{
	FILE* f = fopen(path, "r");
	MausFont* font = calloc(1, sizeof(MausFont));
	char line[1024];

	ule_i32 asc = -1;
	ule_i32 dsc = -1;
	ule_i32 encoding = -1;
	ule_i32 advance = 0;
	ule_i32 bw = 0; /* bmp width */
	ule_i32 bh = 0; /* bmp height */
	ule_i32 xoff = 0;
	ule_i32 yoff = 0;
	ule_i8 seen = 0;

	int i = 0;
	int page = 0;
	int index = 0;

	if (!f) {
		maus_log(stderr, "failed to load font \"%s\"", path);
		return NULL;
	}

	if (!font) {
		maus_log(stderr, "failed to calloc font");
		fclose(f);
		return NULL;
	}

	while (fgets(line, sizeof(line), f)) {
		if (sscanf(line, "FONT_ASCENT %d", &asc) == 1)
			continue;
		if (sscanf(line, "FONT_DESCENT %d", &dsc) == 1)
			continue;

		/* ignore everything until glyphs start */
		if (strncmp(line, "STARTCHAR", 9) != 0)
			continue;

		/* reset vaules for new glyph */
		encoding = -1;
		advance = 0;
		bw = 0;
		bh = 0;
		xoff = 0;
		yoff = 0;

		if (!parse_glyphs(f, font, line, sizeof(line), &encoding, &advance, &bw, &bh, &xoff, &yoff, &seen))
			return NULL;
	}


	fclose(f);

	if (!seen) {
		free(font);
		return NULL;
	}

	/* estimate asc/dsc if they werent provided */
	if (asc < 0 || dsc < 0) {
		asc = 0;
		dsc = 0;

		for (i = 0; i < MAUS_UNICODE_PLANES; i++) {
			if (!font->glyphs[i])
				continue;

			for (page = 0; page < MAUS_GLYPH_PAGE_SIZE; page++) {
				if (!font->glyphs[i][page])
					continue;

				for (index = 0; index < MAUS_GLYPH_PAGE_SIZE; index++) {
					MausGlyph* g = &font->glyphs[i][page][index];

					/* unloaded glpyh */
					if (!g->valid)
						continue;

					/* highest point above baseline */
					if (g->h + g->yoff > asc)
						asc = g->h + g->yoff;

					/* lowest point below baseline */
					if (-g->yoff > dsc)
						dsc = -g->yoff;
				}
			}
		}
	}

	font->asc = asc;
	font->dsc = -dsc;
	font->cellh = asc + dsc;

	if (font->cellw <= 0 || font->cellh <= 0) {
		maus_font_free(font);
		return NULL;
	}

	return font;
}

void maus_font_free(MausFont* font)
{
	int i = 0;
	int page = 0;
	int index = 0;

	if (!font)
		return;

	for (i = 0; i < MAUS_UNICODE_PLANES; i++) {
		if (!font->glyphs[i])
			continue;

		for (page = 0; page < MAUS_GLYPH_PAGE_SIZE; page++) {
			if (!font->glyphs[i][page])
				continue;

			for (index = 0; index < MAUS_GLYPH_PAGE_SIZE; index++)
				free(font->glyphs[i][page][index].bmp);

			free(font->glyphs[i][page]);
		}
		free(font->glyphs[i]);
	}

	free(font);
}

