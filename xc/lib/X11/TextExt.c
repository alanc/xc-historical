/* $XConsortium: XTextExt.c,v 11.13 88/09/06 16:11:05 jim Exp $ */
/************************************************************************
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

************************************************************************/

#define NEED_REPLIES

#include "Xlibint.h"

/* text support routines. Three different access methods, a */
/* charinfo array builder, and a bounding box calculator */

/*ARGSUSED*/
static
XCharStruct *GetCS(min_bounds, pCS, firstCol, numCols, firstRow, numRows, ind, 
	chars, chDefault)
    XCharStruct *min_bounds;
    XCharStruct pCS[];
    unsigned int firstCol, numCols, firstRow, numRows, ind, chDefault;
    unsigned char *chars;
{
    XCharStruct *cs;
    unsigned int c;

    c = chars[ind] - firstCol;
    if (c < numCols) {
  	if ( pCS == NULL ) return min_bounds;
	cs = &pCS[c];
	if (! (cs->attributes & CI_NONEXISTCHAR)) return cs;
    }
    c = chDefault - firstCol;
    if (c >= numCols) return NULL;
    if ( pCS == NULL ) return min_bounds;
    cs = &pCS[c];
    if (! (cs->attributes & CI_NONEXISTCHAR)) return cs;
    return NULL;
}

static
XCharStruct *GetCS2d(min_bounds, pCS, firstCol, numCols, firstRow, numRows, ind, 
	chars, chDefault)
    XCharStruct *min_bounds;
    XCharStruct pCS[];
    unsigned int firstCol, numCols, firstRow, numRows, ind, chDefault;
    unsigned char *chars;
{
    XCharStruct *cs;
    unsigned int row, col, c;

    c = chars[ind] - firstCol;
    if ((firstRow == 0) && (c < numCols)) {
  	if ( pCS == NULL ) return min_bounds;
	cs = &pCS[c];
	if (! (cs->attributes & CI_NONEXISTCHAR)) return cs;
    }
    row = (chDefault >> 8)-firstRow;
    col = (chDefault & 0xff)-firstCol;
    if ((row >= numRows) || (col >= numCols)) return NULL;
    if ( pCS == NULL ) return min_bounds;
    c = row*numCols + col;
    cs = &pCS[c];
    if (! (cs->attributes & CI_NONEXISTCHAR)) return cs;
    return NULL;
}

static void
GetGlyphs(font, count, chars, getGlyph, glyphcount, glyphs)
    XFontStruct *font;
    int count;
    char *chars;
    XCharStruct *(*getGlyph)();
    unsigned int *glyphcount;	/* RETURN */
    XCharStruct *glyphs[];	/* RETURN */
{
    unsigned int    firstCol = font->min_char_or_byte2;
    unsigned int    numCols = font->max_char_or_byte2 - firstCol + 1;
    unsigned int    firstRow = font->min_byte1;
    unsigned int    numRows = font->max_byte1 - firstRow + 1;
    unsigned int    chDefault = font->default_char;
    int		    i, n;
    XCharStruct	    *cs;

    n = 0;
    for (i=0; i < count; i++) {
	cs = (* getGlyph)(
	    &font->min_bounds, font->per_char, firstCol, numCols, firstRow, numRows,
	    i, (unsigned char *) chars, chDefault);
	if (cs != NULL) glyphs[n++] = cs;
    }
    *glyphcount = n;
}

XTextExtents (fontstruct, string, nchars, dir, font_ascent, font_descent,
	           overall)
    XFontStruct *fontstruct;
    register char *string;
    register int nchars;
    int *dir;
    int *font_ascent, *font_descent;
    register XCharStruct *overall;
{
    int	i;
    unsigned int n;

    *dir = fontstruct->direction;
    *font_ascent = fontstruct->max_bounds.ascent;
    *font_descent = fontstruct->max_bounds.descent;

    {
	XCharStruct **charstruct =
	    (XCharStruct **)Xmalloc((unsigned)nchars*sizeof(XCharStruct *));
    
	if (fontstruct->max_byte1 == 0)
	    GetGlyphs(fontstruct, nchars, string, GetCS, &n, charstruct);
	else
	    GetGlyphs(fontstruct, nchars, string, GetCS2d, &n, charstruct);
    
	if (n != 0) {
    
	    overall->ascent  = charstruct[0]->ascent;
	    overall->descent = charstruct[0]->descent;
	    overall->width   = charstruct[0]->width;
	    overall->lbearing    = charstruct[0]->lbearing;
	    overall->rbearing   = charstruct[0]->rbearing;
    
	    for (i=1; i < n; i++) {
		overall->ascent = max(
		    overall->ascent,
		    charstruct[i]->ascent);
		overall->descent = max(
		    overall->descent,
		    charstruct[i]->descent);
		overall->lbearing = min(
		    overall->lbearing,
		    overall->width+charstruct[i]->lbearing);
		overall->rbearing = max(
		    overall->rbearing,
		    overall->width+charstruct[i]->rbearing);
		overall->width += charstruct[i]->width;
	    }
    
	} else {
    
	    overall->ascent  = 0;
	    overall->descent = 0;
	    overall->width   = 0;
	    overall->lbearing = 0;
	    overall->rbearing = 0;
	}
    
	Xfree((char *)charstruct);
    } 
    return (1);
}

int XTextWidth (fontstruct, string, count)
    XFontStruct *fontstruct;
    register char *string;
    int count;
{
    int	i, width;
    unsigned int n;

    {
	XCharStruct **charstruct =
	    (XCharStruct **)Xmalloc((unsigned)count*sizeof(XCharStruct *));
    
	if (fontstruct->max_byte1 == 0)
	    GetGlyphs(fontstruct, count, string, GetCS, &n, charstruct);
	else
	    GetGlyphs(fontstruct, count, string, GetCS2d, &n, charstruct);
    
	if (n != 0) {
 	    width = 0;
	    for (i=0; i < n; i++) {
		width += charstruct[i]->width;
	    }
    
	} else {
	    width   = 0;
	}
    
	Xfree((char *)charstruct);
    
    } 
    return (width);
}

