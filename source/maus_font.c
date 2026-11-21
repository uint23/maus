#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maus.h"
#include "maus_font.h"
#include "uleutf.h"

static MausGlyph* get_glyph(MausFont* font, ule_i32 codepoint, int create);
static void glyph_to_buf(ule_i32 sgx, ule_i32 egx, ule_i32 sgy, ule_i32 egy,
                         ule_i32 gox, ule_i32 goy, MausGlyph* g, ule_u32 col_up,
                         ule_u32* buf, ule_u32 stride);
static ule_i8 parse_glyphs(FILE* f, MausFont* font, char* line, size_t line_size,
                           ule_i32* encoding, ule_i32* advance, ule_i32* bw, ule_i32* bh,
                           ule_i32* xoff, ule_i32* yoff, ule_i8* seen);
static ule_u64 strtou64(const char* str);

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

/* put a glpyh to a buffer */
static void glyph_to_buf(ule_i32 sgx, ule_i32 egx, ule_i32 sgy, ule_i32 egy,
                         ule_i32 gox, ule_i32 goy, MausGlyph* g, ule_u32 col_up,
                         ule_u32* buf, ule_u32 stride)
{
	int gy = 0;
	int gx = 0;
	ule_i32 dstx = 0;
	ule_i32 dsty = 0;

	ule_u32* row = NULL;
	ule_i32 offset = 0;

	for (gy = sgy; gy < egy; gy++) {
		dsty = goy + gy;

		/* calculate row pointer once per scanline */
		row = buf + (dsty * stride);
		offset = gy * g->w;

		for (gx = sgx; gx < egx; gx++) {
			if (g->bmp[offset + gx] > 0) {
				dstx = gox + gx;
				row[dstx] = col_up;
			}
		}
	}
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

					if ((bits >> bit) & 1)
						g->bmp[y * (*bw) + x] = 255; /* solid */
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
	ule_u32 col_up = MAUS_UNPACK_COL(col);

	/* origins to start drawing glyph from */
	ule_i32 gox = 0;
	ule_i32 goy = 0;

	/* starting glyph coordinates */
	ule_i32 sgx = 0;
	ule_i32 sgy = 0;

	/* ending glyph coordinates */
	ule_i32 egx = 0;
	ule_i32 egy = 0;

	/* current glyph */
	MausGlyph* g = 0;

	if (!mw || !mw->bfb || !font)
		return;

	g = get_glyph(font, codepoint, 0);
	if (!g || !g->valid)
		return;

	/* origins to start drawing glyph from */
	gox = x + g->xoff;
	goy = y + font->asc - g->h - g->yoff;

	/* starting coordinates */
	sgx = (-gox > 0) ? -gox : 0;
	sgy = (-goy > 0) ? -goy : 0;

	egx = (mw->width - gox < g->w) ? (mw->width - gox) : g->w;
	egy = (mw->height - goy < g->h) ? (mw->height - goy) : g->h;

	/* cull glyphs which start off screen */
	if (sgx >= egx || sgy >= egy)
		return;

	glyph_to_buf(sgx, egx, sgy, egy, gox, goy, g, col_up, mw->bfb, mw->stride);
}

void maus_draw_text(Maus* mw, MausFont* font, ule_i32 x, ule_i32 y,
                    const char* text, MausColor col)
{
	ule_i32 cp = 0;

	/* current coordinates */
	ule_i32 cx = x;
	ule_i32 cy = y;

	/* current glyph */
	MausGlyph* g = NULL;

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

		g = get_glyph(font, cp, 0);
		if (!g || !g->valid) {
			/* still advance cx if character invalid */
			cx += (font->cellw > 0 ? font->cellw : 8);
			continue;
		}

		cx += g->adv;
	}
}

