/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"

static XRectangle *rects;

static unsigned char bitmap8x8[] = {
    0xCC, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33, 0x99
/*
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
*/
};

static unsigned char bitmap4x4[] = {
   0x03, 0x06, 0x0c, 0x09
};


Bool InitRects(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;
    int size = p->special;
    int x, y;
    int rows;

    rects = (XRectangle *)malloc(p->objects * sizeof(XRectangle));
    x = 0;
    y = 0;
    rows = 0;

    for (i = 0; i != p->objects; i++) {
	rects[i].x = x;
        rects[i].y = y;
	rects[i].width = rects[i].height = size;

	y += size;
	rows++;
	if (y + size > HEIGHT || rows == MAXROWS) {
	    rows = 0;
	    y = 0;
	    x += size;
	    if (x + size > WIDTH) {
		x = 0;
	    }
	}
    }

    if (p->fillStyle == FillStippled || p->fillStyle == FillOpaqueStippled) {
	Pixmap      stipple;
	XGCValues   gcv;

	stipple = XCreateBitmapFromData(xp->d, xp->w, bitmap8x8, 8, 8);
	gcv.stipple = stipple;
	gcv.fill_style = p->fillStyle;
	XChangeGC(xp->d, xp->fggc, GCFillStyle | GCStipple, &gcv);
	XChangeGC(xp->d, xp->bggc, GCFillStyle | GCStipple, &gcv);
	XFreePixmap(xp->d, stipple);

    } else if (p->fillStyle == FillTiled) {
	Pixmap      tile;
	XGCValues   gcv;

	gcv.fill_style = FillTiled;
	tile = XCreatePixmapFromBitmapData(xp->d, xp->w, bitmap4x4, 4, 4,
		    xp->foreground, xp->background,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	gcv.tile = tile;
	XChangeGC(xp->d, xp->fggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(xp->d, tile);
	tile = XCreatePixmapFromBitmapData(xp->d, xp->w, bitmap4x4, 4, 4,
		    xp->background, xp->foreground,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	gcv.fill_style = FillTiled;
	gcv.tile = tile;
	XChangeGC(xp->d, xp->bggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(xp->d, tile);
    }
    return True;
}

void DoRects(xp, p)
    XParms  xp;
    Parms   p;
{
    GC  pgc;
    int i;

    pgc = xp->fggc;
    for (i = 0; i != p->reps; i++) {
        XFillRectangles(xp->d, xp->w, pgc, rects, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void EndRects(xp, p)
    XParms  xp;
    Parms p;
{
    free(rects);
}

