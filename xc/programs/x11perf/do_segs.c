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

Bool InitSegs(xp, p)
    XParms  xp;
    Parms   p;
{
    int size;
    int i;
    int     rows;       /* Number of rows filled in current column      */
    int x, y;		/* base of square to draw in			*/
    int x1, y1;		/* offsets into square				*/
    int x1inc, y1inc;   /* How to get to next x1, y1			*/
    int minorphase;     /* # iterations left with current x1inc, y1inc  */
    int majorphase;     /* count 0..3 for which type of x1inc, y1inc    */

    size = p->special;

    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    size--;		/* Because endcap counts as 1 pixel		*/

    /* All this x, x1, x1inc, etc. stuff is to create a pattern that
	(1) scans down the screen vertically, with each new segment going
	    into a square of size x size.

	(2) rotates the endpoints clockwise around the square

    */

    x     = 0;  y     = 0;
    x1    = 0;  y1    = 0;
    x1inc = 1;  y1inc = 0;
    minorphase = size;
    majorphase = 0;

    rows = 0;

    for (i = 0; i != p->objects; i++) {    
	segments[i].x1 = x + x1;
	segments[i].y1 = y + y1;
	segments[i].x2 = x + size - x1;
	segments[i].y2 = y + size - y1;

	/* Change square to draw segment in */
	rows++;
	y += size + 1;
	if (y >= HEIGHT - size || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    y = 0;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		x = 0;
	    }
	}

	/* Change coordinates of offsets in square */
	x1 += x1inc;
	y1 += y1inc;

	/* Change increments if needed */
	minorphase--;
	if (minorphase <= 0) {
	    minorphase = size;
	    majorphase = (majorphase + 1) % 4;
	    switch (majorphase) {
		case 0: x1inc =  1; y1inc =  0; break;
		case 1: x1inc =  0; y1inc =  1; break;
		case 2: x1inc = -1; y1inc =  0; break;
		case 3: x1inc =  0; y1inc = -1; break;
	    }
	}
    }
    return True;
}
   

Bool InitDashedSegs(xp, p)
    XParms  xp;
    Parms   p;
{
    char dashes[2];

    (void)InitSegs(xp, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(xp->d, xp->bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(xp->d, xp->fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

void DoSegs(xp, p)
    XParms  xp;
    Parms   p;
{
    GC pgc;
    int i;

    pgc = xp->fggc;
    for (i = 0; i != p->reps; i++) {
        XDrawSegments(xp->d, xp->w, pgc, segments, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void EndSegs(xp, p)
    XParms  xp;
    Parms p;
{
    free(segments);
}

