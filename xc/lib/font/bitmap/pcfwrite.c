/*
 * $XConsortium: pcfwrite.c,v 1.4 93/09/17 18:26:53 gildea Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


#include "fntfilst.h"
#include "bitmap.h"
#include "pcf.h"

/* Write PCF font files */

static int  current_position;

static int
pcfWrite(file, b, c)
    FontFilePtr file;
    char       *b;
    int         c;
{
    current_position += c;
    return FontFileWrite(file, b, c);
}

static int
pcfPutLSB32(file, c)
    FontFilePtr file;
{
    current_position += 4;
    (void) FontFilePutc(c, file);
    (void) FontFilePutc(c >> 8, file);
    (void) FontFilePutc(c >> 16, file);
    return FontFilePutc(c >> 24, file);
}

static int
pcfPutINT32(file, format, c)
    FontFilePtr file;
    CARD32      format;
{
    current_position += 4;
    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	(void) FontFilePutc(c >> 24, file);
	(void) FontFilePutc(c >> 16, file);
	(void) FontFilePutc(c >> 8, file);
	return FontFilePutc(c, file);
    } else {
	(void) FontFilePutc(c, file);
	(void) FontFilePutc(c >> 8, file);
	(void) FontFilePutc(c >> 16, file);
	return FontFilePutc(c >> 24, file);
    }
}

static int
pcfPutINT16(file, format, c)
    FontFilePtr file;
    CARD32      format;
{
    current_position += 2;
    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	(void) FontFilePutc(c >> 8, file);
	return FontFilePutc(c, file);
    } else {
	(void) FontFilePutc(c, file);
	return FontFilePutc(c >> 8, file);
    }
}

/*ARGSUSED*/
static int
pcfPutINT8(file, format, c)
    FontFilePtr file;
    CARD32      format;
{
    current_position += 1;
    return FontFilePutc(c, file);
}

static
pcfWriteTOC(file, table, count)
    FontFilePtr file;
    PCFTablePtr table;
    int         count;
{
    CARD32      version;
    int         i;

    version = PCF_FILE_VERSION;
    pcfPutLSB32(file, version);
    pcfPutLSB32(file, count);
    for (i = 0; i < count; i++) {
	pcfPutLSB32(file, table->type);
	pcfPutLSB32(file, table->format);
	pcfPutLSB32(file, table->size);
	pcfPutLSB32(file, table->offset);
	table++;
    }
}

static
pcfPutCompressedMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    pcfPutINT8(file, format, metric->leftSideBearing + 0x80);
    pcfPutINT8(file, format, metric->rightSideBearing + 0x80);
    pcfPutINT8(file, format, metric->characterWidth + 0x80);
    pcfPutINT8(file, format, metric->ascent + 0x80);
    pcfPutINT8(file, format, metric->descent + 0x80);
}

static
pcfPutMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    pcfPutINT16(file, format, metric->leftSideBearing);
    pcfPutINT16(file, format, metric->rightSideBearing);
    pcfPutINT16(file, format, metric->characterWidth);
    pcfPutINT16(file, format, metric->ascent);
    pcfPutINT16(file, format, metric->descent);
    pcfPutINT16(file, format, metric->attributes);
}

static
pcfPutBitmap(file, format, pCI)
    FontFilePtr file;
    CARD32      format;
    CharInfoPtr pCI;
{
    int         count;
    unsigned char *bits;

    count = BYTES_FOR_GLYPH(pCI, PCF_GLYPH_PAD(format));
    bits = (unsigned char *) pCI->bits;
    current_position += count;
    while (count--)
	FontFilePutc(*bits++, file);
}

static
pcfPutAccel(file, format, pFontInfo)
    FontFilePtr file;
    CARD32      format;
    FontInfoPtr pFontInfo;
{
    pcfPutINT8(file, format, pFontInfo->noOverlap);
    pcfPutINT8(file, format, pFontInfo->constantMetrics);
    pcfPutINT8(file, format, pFontInfo->terminalFont);
    pcfPutINT8(file, format, pFontInfo->constantWidth);
    pcfPutINT8(file, format, pFontInfo->inkInside);
    pcfPutINT8(file, format, pFontInfo->inkMetrics);
    pcfPutINT8(file, format, pFontInfo->drawDirection);
    pcfPutINT8(file, format, 0);
    pcfPutINT32(file, format, pFontInfo->fontAscent);
    pcfPutINT32(file, format, pFontInfo->fontDescent);
    pcfPutINT32(file, format, pFontInfo->maxOverlap);
    pcfPutMetric(file, format, &pFontInfo->minbounds);
    pcfPutMetric(file, format, &pFontInfo->maxbounds);
    if (PCF_FORMAT_MATCH(format, PCF_ACCEL_W_INKBOUNDS)) {
	pcfPutMetric(file, format, &pFontInfo->ink_minbounds);
	pcfPutMetric(file, format, &pFontInfo->ink_maxbounds);
    }
}

#define S32 4
#define S16 2
#define S8 1

#define Pad(s)	    (RoundUp(s) - (s))
#define RoundUp(s)  (((s) + 3) & ~3)

#define Compressable(i)	(-128 <= (i) && (i) <= 127)

#define CanCompressMetric(m)	(Compressable((m)->leftSideBearing) && \
				 Compressable((m)->rightSideBearing) && \
				 Compressable((m)->characterWidth) && \
				 Compressable((m)->ascent) && \
				 Compressable((m)->descent) && \
				 (m)->attributes == 0)

#define CanCompressMetrics(min,max) (CanCompressMetric(min) && CanCompressMetric(max))

static char *
pcfNameForAtom(a)
    Atom        a;
{
    return NameForAtom(a);
}

pcfWriteFont(pFont, file)
    FontPtr     pFont;
    FontFilePtr file;
{
    PCFTableRec tables[32],
               *table;
    CARD32      mask,
                bit;
    int         ntables;
    int         size;
    CARD32      format;
    int         i;
    int         cur_table;
    int         prop_string_size;
    int         glyph_string_size;
    xCharInfo  *minbounds,
               *maxbounds;
    xCharInfo  *ink_minbounds,
               *ink_maxbounds;
    BitmapFontPtr  bitmapFont;
    int         nencodings;
    int         header_size;
    FontPropPtr offsetProps;
    int         prop_pad;
    char       *atom_name;
    int         glyph;
    int         offset;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    if (bitmapFont->bitmapExtra) {
	minbounds = &bitmapFont->bitmapExtra->info.minbounds;
	maxbounds = &bitmapFont->bitmapExtra->info.maxbounds;
	ink_minbounds = &bitmapFont->bitmapExtra->info.ink_minbounds;
	ink_maxbounds = &bitmapFont->bitmapExtra->info.ink_maxbounds;
    } else {
	minbounds = &pFont->info.minbounds;
	maxbounds = &pFont->info.maxbounds;
	ink_minbounds = &pFont->info.ink_minbounds;
	ink_maxbounds = &pFont->info.ink_maxbounds;
    }
    offsetProps = (FontPropPtr) xalloc(pFont->info.nprops * sizeof(FontPropRec));
    if (!offsetProps)
	return AllocError;
    prop_string_size = 0;
    for (i = 0; i < pFont->info.nprops; i++) {
	offsetProps[i].name = prop_string_size;
	prop_string_size += strlen(pcfNameForAtom(pFont->info.props[i].name)) + 1;
	if (pFont->info.isStringProp[i]) {
	    offsetProps[i].value = prop_string_size;
	    prop_string_size += strlen(pcfNameForAtom(pFont->info.props[i].value)) + 1;
	} else
	    offsetProps[i].value = pFont->info.props[i].value;
    }
    format = PCF_FORMAT(pFont->bit, pFont->byte, pFont->glyph, pFont->scan);
    mask = 0xFFFFFFF;
    ntables = 0;
    table = tables;
    while (mask) {
	bit = lowbit(mask);
	mask &= ~bit;
	table->type = bit;
	switch (bit) {
	case PCF_PROPERTIES:
	    table->format = PCF_DEFAULT_FORMAT | format;
	    size = S32 + S32 + (S32 + S8 + S32) * pFont->info.nprops;
	    prop_pad = Pad(size);
	    table->size = RoundUp(size) + S32 +
		RoundUp(prop_string_size);
	    table++;
	    break;
	case PCF_ACCELERATORS:
	    if (bitmapFont->bitmapExtra->info.inkMetrics)
		table->format = PCF_ACCEL_W_INKBOUNDS | format;
	    else
		table->format = PCF_DEFAULT_FORMAT | format;
	    table->size = 100;
	    table++;
	    break;
	case PCF_METRICS:
	    if (CanCompressMetrics(minbounds, maxbounds)) {
		table->format = PCF_COMPRESSED_METRICS | format;
		size = S32 + S16 + bitmapFont->num_chars * (5 * S8);
		table->size = RoundUp(size);
	    } else {
		table->format = PCF_DEFAULT_FORMAT | format;
		table->size = S32 + S32 + bitmapFont->num_chars * (6 * S16);
	    }
	    table++;
	    break;
	case PCF_BITMAPS:
	    table->format = PCF_DEFAULT_FORMAT | format;
	    size = S32 + S32 + bitmapFont->num_chars * S32 +
		GLYPHPADOPTIONS * S32 +
		bitmapFont->bitmapExtra->bitmapsSizes[PCF_GLYPH_PAD_INDEX(format)];
	    table->size = RoundUp(size);
	    table++;
	    break;
	case PCF_INK_METRICS:
	    if (bitmapFont->ink_metrics) {
		if (CanCompressMetrics(ink_minbounds, ink_maxbounds)) {
		    table->format = PCF_COMPRESSED_METRICS | format;
		    size = S32 + S16 + bitmapFont->num_chars * (5 * S8);
		    table->size = RoundUp(size);
		} else {
		    table->format = PCF_DEFAULT_FORMAT | format;
		    table->size = S32 + S32 + bitmapFont->num_chars * (6 * S16);
		}
		table++;
	    }
	    break;
	case PCF_BDF_ENCODINGS:
	    table->format = PCF_DEFAULT_FORMAT | format;
	    nencodings = (pFont->info.lastRow - pFont->info.firstRow + 1) *
		(pFont->info.lastCol - pFont->info.firstCol + 1);
	    size = S32 + 5 * S16 + nencodings * S16;
	    table->size = RoundUp(size);
	    table++;
	    break;
	case PCF_SWIDTHS:
	    table->format = PCF_DEFAULT_FORMAT | format;
	    table->size = S32 + S32 + bitmapFont->num_chars * S32;
	    table++;
	    break;
	case PCF_GLYPH_NAMES:
	    table->format = PCF_DEFAULT_FORMAT | format;
	    glyph_string_size = 0;
	    for (i = 0; i < bitmapFont->num_chars; i++)
		glyph_string_size += strlen(pcfNameForAtom(bitmapFont->bitmapExtra->glyphNames[i])) + 1;
	    table->size = S32 + S32 + bitmapFont->num_chars * S32 +
		S32 + RoundUp(glyph_string_size);
	    table++;
	    break;
	case PCF_BDF_ACCELERATORS:
	    if (pFont->info.inkMetrics)
		table->format = PCF_ACCEL_W_INKBOUNDS | format;
	    else
		table->format = PCF_DEFAULT_FORMAT | format;
	    table->size = 100;
	    table++;
	    break;
	}
    }
    ntables = table - tables;
    offset = 0;
    header_size = S32 + S32 + ntables * (4 * S32);
    offset = header_size;
    for (cur_table = 0, table = tables;
	    cur_table < ntables;
	    cur_table++, table++) {
	table->offset = offset;
	offset += table->size;
    }
    current_position = 0;
    pcfWriteTOC(file, tables, ntables);
    for (cur_table = 0, table = tables;
	    cur_table < ntables;
	    cur_table++, table++) {
	if (current_position > table->offset) {
	    printf("can't go backwards... %d > %d\n",
		   current_position, table->offset);
	    return BadFontName;
	}
	while (current_position < table->offset)
	    pcfPutINT8(file, format, '\0');
	pcfPutLSB32(file, table->format);
	switch (table->type) {
	case PCF_PROPERTIES:
	    pcfPutINT32(file, format, pFont->info.nprops);
	    for (i = 0; i < pFont->info.nprops; i++) {
		pcfPutINT32(file, format, offsetProps[i].name);
		pcfPutINT8(file, format, pFont->info.isStringProp[i]);
		pcfPutINT32(file, format, offsetProps[i].value);
	    }
	    for (i = 0; i < prop_pad; i++)
		pcfPutINT8(file, format, 0);
	    pcfPutINT32(file, format, prop_string_size);
	    for (i = 0; i < pFont->info.nprops; i++) {
		atom_name = pcfNameForAtom(pFont->info.props[i].name);
		pcfWrite(file, atom_name, strlen(atom_name) + 1);
		if (pFont->info.isStringProp[i]) {
		    atom_name = pcfNameForAtom(pFont->info.props[i].value);
		    pcfWrite(file, atom_name, strlen(atom_name) + 1);
		}
	    }
	    break;
	case PCF_ACCELERATORS:
	    pcfPutAccel(file, table->format, &bitmapFont->bitmapExtra->info);
	    break;
	case PCF_METRICS:
	    if (PCF_FORMAT_MATCH(table->format, PCF_COMPRESSED_METRICS)) {
		pcfPutINT16(file, format, bitmapFont->num_chars);
		for (i = 0; i < bitmapFont->num_chars; i++)
		    pcfPutCompressedMetric(file, format, &bitmapFont->metrics[i].metrics);
	    } else {
		pcfPutINT32(file, format, bitmapFont->num_chars);
		for (i = 0; i < bitmapFont->num_chars; i++)
		    pcfPutMetric(file, format, &bitmapFont->metrics[i].metrics);
	    }
	    break;
	case PCF_BITMAPS:
	    pcfPutINT32(file, format, bitmapFont->num_chars);
	    glyph = PCF_GLYPH_PAD(format);
	    offset = 0;
	    for (i = 0; i < bitmapFont->num_chars; i++) {
		pcfPutINT32(file, format, offset);
		offset += BYTES_FOR_GLYPH(&bitmapFont->metrics[i], glyph);
	    }
	    for (i = 0; i < GLYPHPADOPTIONS; i++) {
		pcfPutINT32(file, format,
			    bitmapFont->bitmapExtra->bitmapsSizes[i]);
	    }
	    for (i = 0; i < bitmapFont->num_chars; i++)
		pcfPutBitmap(file, format, &bitmapFont->metrics[i]);
	    break;
	case PCF_INK_METRICS:
	    if (PCF_FORMAT_MATCH(table->format, PCF_COMPRESSED_METRICS)) {
		pcfPutINT16(file, format, bitmapFont->num_chars);
		for (i = 0; i < bitmapFont->num_chars; i++)
		    pcfPutCompressedMetric(file, format, &bitmapFont->ink_metrics[i]);
	    } else {
		pcfPutINT32(file, format, bitmapFont->num_chars);
		for (i = 0; i < bitmapFont->num_chars; i++)
		    pcfPutMetric(file, format, &bitmapFont->ink_metrics[i]);
	    }
	    break;
	case PCF_BDF_ENCODINGS:
	    pcfPutINT16(file, format, pFont->info.firstCol);
	    pcfPutINT16(file, format, pFont->info.lastCol);
	    pcfPutINT16(file, format, pFont->info.firstRow);
	    pcfPutINT16(file, format, pFont->info.lastRow);
	    pcfPutINT16(file, format, pFont->info.defaultCh);
	    for (i = 0; i < nencodings; i++) {
		if (bitmapFont->encoding[i])
		    pcfPutINT16(file, format, bitmapFont->encoding[i] - bitmapFont->metrics);
		else
		    pcfPutINT16(file, format, 0xFFFF);
	    }
	    break;
	case PCF_SWIDTHS:
	    pcfPutINT32(file, format, bitmapFont->num_chars);
	    for (i = 0; i < bitmapFont->num_chars; i++)
		pcfPutINT32(file, format, bitmapFont->bitmapExtra->sWidths[i]);
	    break;
	case PCF_GLYPH_NAMES:
	    pcfPutINT32(file, format, bitmapFont->num_chars);
	    offset = 0;
	    for (i = 0; i < bitmapFont->num_chars; i++) {
		pcfPutINT32(file, format, offset);
		offset += strlen(pcfNameForAtom(bitmapFont->bitmapExtra->glyphNames[i])) + 1;
	    }
	    pcfPutINT32(file, format, offset);
	    for (i = 0; i < bitmapFont->num_chars; i++) {
		atom_name = pcfNameForAtom(bitmapFont->bitmapExtra->glyphNames[i]);
		pcfWrite(file, atom_name, strlen(atom_name) + 1);
	    }
	    break;
	case PCF_BDF_ACCELERATORS:
	    pcfPutAccel(file, table->format, &pFont->info);
	    break;
	}
    }
    return Successful;
}
