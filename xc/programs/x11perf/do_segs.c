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

static XSegment *segments;
static GC       pgc;

int InitSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int size;
    int     half;
    int i;
    int     rows;       /* Number of rows filled in current column      */
    int x, y;		/* base of square to draw in			*/
    int x1, y1, x2, y2;	/* offsets into square				*/
    int phase;		/* how far into 0..8*size we are		*/
    int phaseinc;       /* how much to increment phase at each segment  */
    int size8;		/* 8 * size					*/
    XGCValues   gcv;

    pgc = xp->fggc;

    size = p->special;
    size8 = 8 * size;
    half = (size + 19) / 20;

    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    /* All this x, x1, etc. stuff is to create a pattern that
	(1) scans down the screen vertically, with each new segment going
	    into a square of size^2.

	(2) rotates the endpoints clockwise around the square

	(3) rotates by ``large'' steps if we aren't going to paint enough
	    segments to get full coverage

	(4) uses CapNotLast so we can create segments of length 1 that
	    nonetheless have a distinct direction
    */

    x     = half;  y     = half;
    phase = 0;
    phaseinc = size8 / p->objects;
    if (phaseinc == 0) phaseinc = 1;
    rows = 0;

    for (i = 0; i != p->objects; i++) {    
	switch (phase / size) {
	case 0:
	    x1 = 0;
	    y1 = 0;
	    x2 = size;
	    y2 = phase;
	    break;

	case 1:
	    x1 = phase % size;    
	    y1 = 0;
	    x2 = size;
	    y2 = size;
	    break;

	case 2:
	    x1 = size;
	    y1 = 0;
	    x2 = size - phase % size;
	    y2 = size;
	    break;

	case 3:
	    x1 = size;
	    y1 = phase % size;
	    x2 = 0;
	    y2 = size;
	    break;

	case 4:
	    x1 = size;
	    y1 = size;
	    x2 = 0;
	    y2 = size - phase % size;
	    break;

	case 5:
	    x1 = size - phase % size;
	    y1 = size;
	    x2 = 0;
	    y2 = 0;
	    break;

	case 6:
	    x1 = 0;
	    y1 = size;
	    x2 = phase % size;
	    y2 = 0;
	    break;

	case 7:
	    x1 = 0;
	    y1 = size - phase % size;
	    x2 = size;
	    y2 = 0;
	    break;
	} /* end switch */

	segments[i].x1 = x + x1;
	segments[i].y1 = y + y1;
	segments[i].x2 = x + x2;
	segments[i].y2 = y + y2;

	/* Change square to draw segment in */
	rows++;
	y += size;
	if (y >= HEIGHT - size - half || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    y = half;
	    x += size;
	    if (x >= WIDTH - size - half) {
		x = half;
	    }
	}

	/* Increment phase */
	phase += phaseinc;
	if (phase >= size8) phase -= size8;

    }

    gcv.cap_style = CapNotLast;
    XChangeGC(xp->d, xp->fggc, GCCapStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCCapStyle, &gcv);
    
    return reps;
}
   

int InitDashedSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    char dashes[2];

    (void)InitSegments(xp, p, reps);

    /* Modify GCs to draw dashed */
    XSetLineAttributes
	(xp->d, xp->bggc, 0, LineOnOffDash, CapNotLast, JoinMiter);
    XSetLineAttributes
	(xp->d, xp->fggc, 0, LineOnOffDash, CapNotLast, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return reps;
}

int InitDoubleDashedSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    char dashes[2];

    (void)InitSegments(xp, p, reps);

    /* Modify GCs to draw dashed */
    XSetLineAttributes
	(xp->d, xp->bggc, 0, LineDoubleDash, CapNotLast, JoinMiter);
    XSetLineAttributes
	(xp->d, xp->fggc, 0, LineDoubleDash, CapNotLast, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return reps;
}

int InitHorizSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int size;
    int     half;
    int i;
    int x, y;		/* base of square to draw in			*/
    int phase;		/* how far into 0..8*size we are		*/
    int phaseinc;       /* how much to increment phase at each segment  */
    XGCValues   gcv;

    pgc = xp->fggc;

    size = p->special;
    half = (size + 19) / 20;

    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    x = half;
    y = half;
    for (i = 0; i != p->objects; i++) {
	segments[i].x1 = x;
	segments[i].y1 = y;
	segments[i].x2 = x + size;
	segments[i].y2 = y;
	y += 2;
	if (y >= HEIGHT - size - half) {
	    y = half;
	    x += size + 1;
	    if (x >= WIDTH - size - half)
		x = half;
	}
    }
    gcv.cap_style = CapNotLast;
    XChangeGC(xp->d, xp->fggc, GCCapStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCCapStyle, &gcv);
    return reps;
}

int InitVertSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int size;
    int     half;
    int i;
    int x, y;		/* base of square to draw in			*/
    int phase;		/* how far into 0..8*size we are		*/
    int phaseinc;       /* how much to increment phase at each segment  */
    XGCValues   gcv;

    pgc = xp->fggc;

    size = p->special;
    half = (size + 19) / 20;

    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    x = half;
    y = half;
    for (i = 0; i != p->objects; i++) {
	segments[i].x1 = x;
	segments[i].y1 = y;
	segments[i].x2 = x;
	segments[i].y2 = y + size;
	y += size + 1;
	if (y >= HEIGHT - size - half) {
	    y = half;
	    x += 2;
	    if (x >= WIDTH - size - half) {
		x = half;
	    }
	}
    }
    gcv.cap_style = CapNotLast;
    XChangeGC(xp->d, xp->fggc, GCCapStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCCapStyle, &gcv);
    return reps;
}

void DoSegments(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i;

    for (i = 0; i != reps; i++) {
        XDrawSegments(xp->d, xp->w, pgc, segments, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void EndSegments(xp, p)
    XParms  xp;
    Parms p;
{
    free(segments);
}

