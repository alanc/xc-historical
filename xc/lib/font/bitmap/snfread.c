/************************************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

/* $XConsortium: snfread.c,v 1.6 91/05/30 19:06:43 keith Exp $ */

#include <ctype.h>
#include "fontfilest.h"
#include "bitmap.h"
#include "snfstr.h"

void        snfUnloadFont();

static int
snfReadCharInfo(file, charInfo, base)
    FontFilePtr file;
    CharInfoPtr charInfo;
    char       *base;
{
    snfCharInfoRec snfCharInfo;

#define Width(m)    ((m).rightSideBearing - (m).leftSideBearing)
#define Height(m)   ((m).ascent + (m).descent)

    if (FontFileRead(file, (char *) &snfCharInfo, sizeof snfCharInfo) !=
	    sizeof(snfCharInfo)) {
	return BadFontName;
    }
    charInfo->metrics = snfCharInfo.metrics;
    if (snfCharInfo.exists)
	charInfo->bits = base + snfCharInfo.byteOffset;
    else
	charInfo->bits = 0;
    return Successful;
}

static int
snfReadxCharInfo(file, charInfo)
    FontFilePtr file;
    xCharInfo  *charInfo;
{
    snfCharInfoRec snfCharInfo;

    if (FontFileRead(file, (char *) &snfCharInfo, sizeof snfCharInfo) !=
	    sizeof(snfCharInfo)) {
	return BadFontName;
    }
    *charInfo = snfCharInfo.metrics;
    return Successful;
}

static
snfCopyInfo(snfInfo, pFontInfo)
    snfFontInfoPtr snfInfo;
    FontInfoPtr pFontInfo;
{
    pFontInfo->firstCol = snfInfo->firstCol;
    pFontInfo->lastCol = snfInfo->lastCol;
    pFontInfo->firstRow = snfInfo->firstRow;
    pFontInfo->lastRow = snfInfo->lastRow;
    pFontInfo->defaultCh = snfInfo->chDefault;
    pFontInfo->noOverlap = snfInfo->noOverlap;
    pFontInfo->terminalFont = snfInfo->terminalFont;
    pFontInfo->constantMetrics = snfInfo->constantMetrics;
    pFontInfo->constantWidth = snfInfo->constantWidth;
    pFontInfo->inkInside = snfInfo->inkInside;
    pFontInfo->inkMetrics = snfInfo->inkMetrics;
    pFontInfo->allExist = snfInfo->allExist;
    pFontInfo->drawDirection = snfInfo->drawDirection;
    pFontInfo->maxOverlap = 0;
    pFontInfo->minbounds = snfInfo->minbounds.metrics;
    pFontInfo->maxbounds = snfInfo->maxbounds.metrics;
    pFontInfo->fontAscent = snfInfo->fontAscent;
    pFontInfo->fontDescent = snfInfo->fontDescent;
    pFontInfo->nprops = snfInfo->nProps;
}

static int
snfReadProps(snfInfo, pFontInfo, file)
    snfFontInfoPtr snfInfo;
    FontInfoPtr pFontInfo;
    FontFilePtr file;
{
    char       *strings;
    FontPropPtr pfp;
    snfFontPropPtr psnfp;
    char       *propspace;
    int         bytestoalloc;
    int         i;

    bytestoalloc = snfInfo->nProps * sizeof(snfFontPropRec) +
	BYTESOFSTRINGINFO(snfInfo);
    propspace = (char *) xalloc(bytestoalloc);
    if (!propspace)
	return AllocError;

    if (FontFileRead(file, propspace, bytestoalloc) != bytestoalloc) {
	xfree(propspace);
	return BadFontName;
    }
    psnfp = (snfFontPropPtr) propspace;

    strings = propspace + BYTESOFPROPINFO(snfInfo);

    for (i = 0, pfp = pFontInfo->props; i < snfInfo->nProps; i++, pfp++, psnfp++) {
	pfp->name = MakeAtom(&strings[psnfp->name],
			     (unsigned) strlen(&strings[psnfp->name]), 1);
	pFontInfo->isStringProp[i] = psnfp->indirect;
	if (psnfp->indirect)
	    pfp->value = (INT32) MakeAtom(&strings[psnfp->value],
			       (unsigned) strlen(&strings[psnfp->value]), 1);
	else
	    pfp->value = psnfp->value;
    }

    xfree(propspace);
    return Successful;
}

int
snfReadHeader(snfInfo, file)
    snfFontInfoPtr snfInfo;
    FontFilePtr file;
{
    if (FontFileRead(file, (char *) snfInfo, sizeof *snfInfo) != sizeof *snfInfo)
	return BadFontName;

    if (snfInfo->version1 != FONT_FILE_VERSION ||
	    snfInfo->version2 != FONT_FILE_VERSION)
	return BadFontName;
    return Successful;
}

int
snfReadFont(pFont, file, bit, byte, glyph, scan)
    FontPtr     pFont;
    FontFilePtr file;
    int         bit,
                byte,
                glyph,
                scan;
{
    snfFontInfoRec fi;
    unsigned    bytestoalloc;
    int         i;
    char       *fontspace;
    BitmapFontPtr  bitmapFont;
    int         num_chars;
    int         bitmapsSize;
    int         ret;
    int         metrics_off;
    int         bitmaps_off;
    int         encoding_off;
    int         props_off;
    int         isStringProp_off;
    int         ink_off;

    ret = snfReadHeader(&fi, file);
    if (ret != Successful)
	return ret;

    /*
     * we'll allocate one chunk of memory and split it among the various parts
     * of the font:
     * 
     * BitmapFontRec CharInfoRec's Glyphs Encoding DIX Properties Ink CharInfoRec's
     */

    bitmapsSize = BYTESOFGLYPHINFO(&fi);
    num_chars = n2dChars(&fi);
    bytestoalloc = sizeof(BitmapFontRec);	/* bitmapFont */
    metrics_off = bytestoalloc;
    bytestoalloc += num_chars * sizeof(CharInfoRec);	/* metrics */
    bitmaps_off = bytestoalloc;
    bytestoalloc += bitmapsSize;/* bitmaps */
    bytestoalloc = (bytestoalloc + 3) & ~3;
    encoding_off = bytestoalloc;
    bytestoalloc += num_chars * sizeof(CharInfoPtr);	/* encoding */
    props_off = bytestoalloc;
    bytestoalloc += fi.nProps * sizeof(FontPropRec);	/* props */
    isStringProp_off = bytestoalloc;
    bytestoalloc += fi.nProps * sizeof(char);	/* isStringProp */
    bytestoalloc = (bytestoalloc + 3) & ~3;
    ink_off = bytestoalloc;
    if (fi.inkMetrics)
	bytestoalloc += num_chars * sizeof(xCharInfo);	/* ink_metrics */

    fontspace = (char *) xalloc(bytestoalloc);
    if (!fontspace)
	return AllocError;

    /*
     * now fix up pointers
     */

    bitmapFont = (BitmapFontPtr) fontspace;
    bitmapFont->num_chars = num_chars;
    bitmapFont->metrics = (CharInfoPtr) (fontspace + metrics_off);
    bitmapFont->bitmaps = (char *) (fontspace + bitmaps_off);
    bitmapFont->encoding = (CharInfoPtr *) (fontspace + encoding_off);
    bitmapFont->pDefault = NULL;
    bitmapFont->bitmapExtra = NULL;
    pFont->info.props = (FontPropPtr) (fontspace + props_off);
    pFont->info.isStringProp = (char *) (fontspace + isStringProp_off);
    if (fi.inkMetrics)
	bitmapFont->ink_metrics = (xCharInfo *) (fontspace + ink_off);
    else
	bitmapFont->ink_metrics = 0;

    /*
     * read the CharInfo
     */

    ret = Successful;
    for (i = 0; ret == Successful && i < num_chars; i++) {
	ret = snfReadCharInfo(file, &bitmapFont->metrics[i], bitmapFont->bitmaps);
	if (bitmapFont->metrics[i].bits)
	    bitmapFont->encoding[i] = &bitmapFont->metrics[i];
	else
	    bitmapFont->encoding[i] = 0;
    }

    if (ret != Successful) {
	xfree(fontspace);
	return ret;
    }
    /*
     * read the glyphs
     */

    if (FontFileRead(file, (char *) bitmapFont->bitmaps, bitmapsSize) !=
	    bitmapsSize) {
	xfree(fontspace);
	return BadFontName;
    }
    /* now read and atom'ize properties */

    ret = snfReadProps(&fi, &pFont->info, file);
    if (ret != Successful) {
	xfree(fontspace);
	return ret;
    }
    snfCopyInfo(&fi, &pFont->info);

    /* finally, read the ink metrics if the exist */

    if (fi.inkMetrics) {
	ret = Successful;
	ret = snfReadxCharInfo(file, &pFont->info.ink_minbounds);
	ret = snfReadxCharInfo(file, &pFont->info.ink_maxbounds);
	for (i = 0; ret == Successful && i < num_chars; i++)
	    ret = snfReadxCharInfo(file, &bitmapFont->ink_metrics[i]);
	if (ret != Successful) {
	    xfree(fontspace);
	    return ret;
	}
    } else {
	pFont->info.ink_minbounds = pFont->info.minbounds;
	pFont->info.ink_maxbounds = pFont->info.maxbounds;
    }

    if (pFont->info.defaultCh != (unsigned short) NO_SUCH_CHAR) {
	int         r,
	            c,
	            cols;

	r = pFont->info.defaultCh >> 8;
	c = pFont->info.defaultCh & 0xFF;
	if (pFont->info.firstRow <= r && r <= pFont->info.lastRow &&
		pFont->info.firstCol <= c && c <= pFont->info.lastCol) {
	    cols = pFont->info.lastCol - pFont->info.firstCol + 1;
	    r = r - pFont->info.firstRow;
	    c = c - pFont->info.firstCol;
	    bitmapFont->pDefault = &bitmapFont->metrics[r * cols + c];
	}
    }
    bitmapFont->bitmapExtra = (BitmapExtraPtr) 0;
    pFont->fontPrivate = (pointer) bitmapFont;
    pFont->get_bitmaps = bitmapGetBitmaps;
    pFont->get_extents = bitmapGetExtents;
    pFont->get_glyphs = bitmapGetGlyphs;
    pFont->get_metrics = bitmapGetMetrics;
    pFont->unload_font = snfUnloadFont;
    pFont->bit = bit;
    pFont->byte = byte;
    pFont->glyph = glyph;
    pFont->scan = scan;
    return Successful;
}

int
snfReadFontInfo(pFontInfo, file)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
{
    int         ret;
    snfFontInfoRec fi;
    int         bytestoskip;
    int         num_chars;

    ret = snfReadHeader(&fi, file);
    if (ret != Successful)
	return ret;
    snfCopyInfo(&fi, pFontInfo);

    pFontInfo->props = (FontPropPtr) xalloc(fi.nProps * sizeof(FontPropRec));
    if (!pFontInfo->props)
	return AllocError;
    pFontInfo->isStringProp = (char *) xalloc(fi.nProps * sizeof(char));
    if (!pFontInfo->isStringProp) {
	xfree(pFontInfo->props);
	return AllocError;
    }
    num_chars = n2dChars(&fi);
    bytestoskip = num_chars * sizeof(snfCharInfoRec);	/* charinfos */
    bytestoskip += BYTESOFGLYPHINFO(&fi);
    FontFileSkip(file, bytestoskip);

    ret = snfReadProps(&fi, pFontInfo, file);
    if (ret != Successful) {
	xfree(pFontInfo->props);
	xfree(pFontInfo->isStringProp);
	return ret;
    }
    if (fi.inkMetrics) {
	ret = snfReadxCharInfo(file, &pFontInfo->ink_minbounds);
	if (ret != Successful) {
	    xfree(pFontInfo->props);
	    xfree(pFontInfo->isStringProp);
	    return ret;
	}
	ret = snfReadxCharInfo(file, &pFontInfo->ink_maxbounds);
	if (ret != Successful) {
	    xfree(pFontInfo->props);
	    xfree(pFontInfo->isStringProp);
	    return ret;
	}
    }
    return Successful;

}

static void
snfUnloadFont(pFont)
    FontPtr	    pFont;
{
    xfree(pFont->fontPrivate);
}
