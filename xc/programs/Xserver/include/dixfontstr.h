/* $XConsortium: dixfontstr.h,v 1.6 89/03/11 15:20:13 rws Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

******************************************************************/

#ifndef DIXFONTSTRUCT_H
#define DIXFONTSTRUCT_H

#include "dixfont.h"
#include "fontstruct.h"
#include "misc.h"

extern FontPtr	FontFileLoad(/* name, length */); /* implemented in OS layer */
extern Bool	FontFilePropLoad(/* name, length, *font, fi, *props */);
extern void	FontUnload(/* font */);

#ifndef R4_FONT_STRUCTURES

#define FONTCHARSET(font)	  (font)->pCS
#define FONTMAXBOUNDS(font,field) (font)->pCS->maxbounds.field
#define FONTMINBOUNDS(font,field) (font)->pCS->minbounds.field
#define TERMINALFONT(font)	  (font)->pCS->terminalFont
#define FONTASCENT(font)	  (font)->pCS->fontAscent
#define FONTDESCENT(font)	  (font)->pCS->fontDescent
#define FONTGLYPHS(font)	  (font)->pCS->pBitmaps
#define FONTCONSTMETRICS(font)	  (font)->pCS->constantMetrics
#define FONTCONSTWIDTH(font)	  (font)->pCS->constantWidth
#define FONTALLEXIST(font)	  (font)->allExist
#define FONTFIRSTCOL(font)	  (font)->firstCol
#define FONTLASTCOL(font)	  (font)->lastCol
#define FONTFIRSTROW(font)	  (font)->firstRow
#define FONTLASTROW(font)	  (font)->lastRow
#define FONTDEFAULTCH(font)	  (font)->defaultCh
#define FONTINKMIN(font)	  (&((font)->inkMin))
#define FONTINKMAX(font)	  (&((font)->inkMax))
#define FONTPROPS(font)		  (font)->pCS->props
#define FONTGLYPHBITS(base,pci)	  ((unsigned char *) (pci)->pPriv)

extern FontPathPtr  fpExpandFontNamePattern(), fpGetFontPath();

typedef struct _EncodedFont FontRec;
typedef struct _CharSet	    FontInfoRec, *FontInfoPtr;

#else

typedef struct _DIXFontProp {
    ATOM	name;
    INT32	value;	/* assumes ATOM is not larger than INT32 */
} DIXFontProp;

/*
 * FONT is created at font load time; it is not part of the
 * font file format.
 */
typedef struct _Font {
    FontInfoPtr	pFI;
    DIXFontProp	*pFP;
    CharInfoPtr	pCI;			/* bitmap metrics and offset */
    char	*pGlyphs;
    pointer	osPrivate;
    int		fileType;		/* tag for OS layer */
    int		refcnt;			/* free storage when this goes to 0 */
    pointer	devPriv[MAXSCREENS];	/* information private to screen */
    CharInfoPtr	pInkCI;			/* ink metrics */
    CharInfoPtr	pInkMin;		/* ink metrics */
    CharInfoPtr	pInkMax;		/* ink metrics */
} FontRec;

#define FONTCHARSET(font)	  (font)->pFI
#define FONTMAXBOUNDS(font,field) (font)->pFI->maxbounds.metrics.field
#define FONTMINBOUNDS(font,field) (font)->pFI->minbounds.metrics.field
#define TERMINALFONT(font)	  (font)->pFI->terminalFont
#define FONTASCENT(font)	  (font)->pFI->fontAscent
#define FONTDESCENT(font)	  (font)->pFI->fontDescent
#define FONTGLYPHS(font)	  (font)->pGlyphs
#define FONTCONSTMETRICS(font)	  (font)->pFI->constantMetrics
#define FONTCONSTWIDTH(font)	  (font)->pFI->constantWidth
#define FONTALLEXIST(font)	  (font)->pFI->allExist
#define FONTFIRSTCOL(font)	  (font)->pFI->firstCol
#define FONTLASTCOL(font)	  (font)->pFI->lastCol
#define FONTFIRSTROW(font)	  (font)->pFI->firstRow
#define FONTLASTROW(font)	  (font)->pFI->lastRow
#define FONTDEFAULTCH(font)	  (font)->pCI->chDefault
#define FONTHASINK(font)	  (font)->pFI->inkMetrics
#define FONTINKMIN(font)	  (&(font)->pInkMin.metrics)
#define FONTINKMAX(font)	  (&(font)->pInkMax.metrics)
#define FONTPROPS(font)		  (font)->pFP
#define FONTGLYPHBITS(base,pci)	  (((unsigned char *) base) + (pci)->byteOffset)

typedef struct _FontInfoRec	FontInfoRec, *FontInfoPtr;
typedef struct _DIXFontProp	DIXFontPropRec, *DIXFontPropPtr;

#endif

/* some things haven't changed names, but we'll be careful anyway */

#define FONTREFCNT(font)	  (font)->refcnt
#define FONTINFONPROPS(pfi)	  (pfi)->nProps

/*
 * for linear char sets
 */
#define N1dChars(pfont)	(FONTLASTCOL(pfont) - FONTFIRSTCOL(pfont) + 1)

/*
 * for 2D char sets
 */
#define N2dChars(pfont)	(N1dChars(pfont) * \
			 (FONTLASTROW(pfont) - FONTFIRSTROW(pfont) + 1))


/* in dixfont.c */
extern Bool	SetDefaultFont();
extern int	CloseFont();
extern Bool	DescribeFont();
extern void	ServerBitmapFromGlyph();
extern Bool	CursorMetricsFromGlyph();
extern void	GetGlyphs();
extern Bool	QueryTextExtents();

#endif /* DIXFONTSTRUCT_H */
