/* $XConsortium: fsconvert.c,v 1.4 91/05/14 15:40:29 rws Exp $ */
/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Network Computing
 * Devices makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 *
 */
/*
 * FS data conversion
 */

#include	"fontmisc.h"
#include	"fontstruct.h"
#include	"FS.h"
#include	"FSproto.h"
#include	"fservestr.h"

/*
 * converts data from font server form to X server form
 */

void
fs_convert_char_info(src, ci)
    fsCharInfo *src;
    CharInfoPtr ci;
{
    xCharInfo  *dst = &ci->metrics;

    dst->ascent = src->ascent;
    dst->descent = src->descent;
    dst->leftSideBearing = src->left;
    dst->rightSideBearing = src->right;
    dst->characterWidth = src->width;
    dst->attributes = src->attributes;
}

int
fs_convert_header(hdr, pfi)
    fsFontHeader *hdr;
    FontInfoPtr pfi;
{
    pfi->allExist = (hdr->flags & FontInfoAllCharsExist) != 0;
    pfi->drawDirection = (hdr->draw_direction == LeftToRightDrawDirection) ?
	LeftToRight : RightToLeft;
    pfi->inkInside = (hdr->flags & FontInfoInkInside) != 0;
    pfi->firstCol = hdr->char_range.min_char.low;
    pfi->firstRow = hdr->char_range.min_char.high;
    pfi->lastCol = hdr->char_range.max_char.low;
    pfi->lastRow = hdr->char_range.max_char.high;

    pfi->defaultCh = hdr->default_char.low + (hdr->default_char.high << 8);

    pfi->fontDescent = hdr->font_descent;
    pfi->fontAscent = hdr->font_ascent;


    fs_convert_char_info(&hdr->min_bounds, &pfi->minbounds);
    fs_convert_char_info(&hdr->max_bounds, &pfi->maxbounds);

    fs_convert_char_info(&hdr->min_bounds, &pfi->ink_minbounds);
    fs_convert_char_info(&hdr->max_bounds, &pfi->ink_maxbounds);

    FontComputeInfoAccelerators (pfi);

    return 1;
}

int
fs_convert_props(pi, po, pd, pfi)
    fsPropInfo *pi;
    fsPropOffset *po;
    pointer     pd;
    FontInfoPtr pfi;
{
    FontPropPtr dprop;
    int         i,
                nprops;
    char       *is_str;

    nprops = pfi->nprops = pi->num_offsets;

    dprop = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    is_str = (char *) xalloc(sizeof(char) * nprops);
    if (!dprop || !is_str) {
	xfree(is_str);
	xfree(dprop);
	return -1;
    }
    pfi->props = dprop;
    pfi->isStringProp = is_str;

    for (i = 0; i < nprops; i++, dprop++, po++, is_str++) {
	dprop->name = MakeAtom(&pd[po->name.position], po->name.length, 1);
	if (po->type != PropTypeString) {
	    *is_str = FALSE;
	    dprop->value = po->value.position;
	} else {
	    *is_str = TRUE;
	    dprop->value = (INT32) MakeAtom(&pd[po->value.position],
					    po->value.length, 1);
	}
    }

    return nprops;
}

int
fs_convert_lfwi_reply(pfi, fsrep, pi, po, pd)
    FontInfoPtr pfi;
    fsListFontsWithXInfoReply *fsrep;
    fsPropInfo *pi;
    fsPropOffset *po;
    pointer     pd;
{
    fsFontHeader *hdr = &fsrep->header;

    fs_convert_header(hdr, pfi);
    fs_convert_props(pi, po, pd, pfi);

    return Successful;
}

/*
 * figures out what glyphs to request
 * this is where lots of extra
 * smarts wants to live
 */
/* ARGSUSED */
int
fs_build_range(pfont, count, item_size, range, data)
    FontPtr     pfont;
    unsigned int count;
    int         item_size;
    fsRange    *range;
    unsigned char *data;
{
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);

    if (fsd->complete)
	return AccessDone;
    else
	return Successful;
}

/*
 * figures out what extents to request
 * this is where lots of extra
 * smarts wants to live
 */
/* ARGSUSED */
int
fs_check_extents(pfont, flags, nranges, range, blockrec)
    FontPtr     pfont;
    Mask        flags;
    int         nranges;
    fsRange    *range;
    FSBlockDataPtr blockrec;
{
/* XXX -- either fill in the requested info if we have it somewhere
 * and return AccessDone, or else return Successful
 */
    return Successful;
}

/*
 * figures out what glyphs to request
 * this is where lots of extra
 * smarts wants to live
 */
/* ARGSUSED */
int
fs_check_bitmaps(pfont, format, flags, nranges, range, blockrec)
    FontPtr     pfont;
    fsBitmapFormat format;
    Mask        flags;
    int         nranges;
    fsRange    *range;
    FSBlockDataPtr blockrec;
{
/* XXX -- either fill in the requested info if we have it somewhere
 * and return AccessDone, or else return Successful
 */
    return Successful;
}

static int
_fs_get_glyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    FSFontPtr   fsdata;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    FSFontDataPtr fsd = (FSFontDataPtr) pFont->fpePrivate;
    int         itemSize;
    int         err = Successful;

    fsdata = (FSFontPtr) pFont->fontPrivate;
    encoding = fsdata->encoding;
    pDefault = fsdata->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;


    /* XXX - this should be much smarter */
    /* make sure the glyphs are there */
    if (charEncoding == Linear8Bit || charEncoding == TwoD8Bit)
	itemSize = 1;
    else
	itemSize = 2;
    if (!fsd->complete)
	err = fs_load_glyphs((pointer) 0, pFont, count, itemSize, chars);
    if (err != Successful)
	return err;

    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    (pci = &encoding[r * numCols + c])->bits)
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}

static CharInfoRec junkDefault;

static int
_fs_get_metrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    xCharInfo **glyphs;		/* RETURN */
{
    int         ret;
    FSFontPtr   fsfont;
    int         i;

    fsfont = (FSFontPtr) pFont->fontPrivate;
    if (!fsfont->pDefault)
	fsfont->pDefault = &junkDefault;
    ret = _fs_get_glyphs(pFont, count, chars, charEncoding,
			 glyphCount, (CharInfoPtr *) glyphs);

    if (ret == Successful) {
	if (fsfont->pDefault == &junkDefault) {
	    for (i = 0; i < *glyphCount; i++) {
		if (glyphs[i] == (xCharInfo *) & junkDefault)
		    glyphs[i] = 0;
	    }
	}
    }
    if (fsfont->pDefault == &junkDefault)
	fsfont->pDefault = 0;
    return ret;
}

static void
fs_unload_font(pfont)
    FontPtr     pfont;
{
    FSFontPtr   fsdata = (FSFontPtr) pfont->fontPrivate;

    xfree(fsdata->encoding);
    xfree(fsdata->bitmaps);
    xfree(fsdata);

    pfont->fontPrivate = 0;
}

void
fs_init_font(pfont)
    FontPtr     pfont;
{
    /* set font function pointers */
    extern int  fs_load_bitmaps();
    extern int  fs_load_extents();

    pfont->get_glyphs = _fs_get_glyphs;
    pfont->get_metrics = _fs_get_metrics;
    pfont->get_bitmaps = fs_load_bitmaps;
    pfont->get_extents = fs_load_extents;
    pfont->unload_font = fs_unload_font;
}
