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

static XArc *arcs;

Bool InitCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i;
    int     rows;       /* Number of rows filled in current column	    */
    int     x, y;       /* base of square to draw the circle in		    */
    int     xorg, yorg; /* Used to get from column to column or row to row  */
    int     size;

    size = p->special;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    xorg = 0; yorg = 0;
    x    = 0; y    = 0;
    rows = 0;

    for (i = 0; i != p->objects; i++) {    
	arcs[i].x = x;
	arcs[i].y = y;
	arcs[i].width = size;
	arcs[i].height = size;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;

	y += size + 1;
	rows++;
	if (y >= HEIGHT - size || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    yorg++;
	    if (yorg >= size || yorg >= HEIGHT - size) {
		yorg = 0;
		xorg++;
		if (xorg >= size || xorg >= WIDTH - size) {
		    xorg = 0;
		}
	    }
	    y = yorg;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		x = xorg;
	    }
	}
    }
    return True;
}

Bool InitDashedCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    char dashes[2];

    (void)InitCircles(xp, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(xp->d, xp->bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(xp->d, xp->fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

Bool InitDoubleDashedCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    char dashes[2];

    (void)InitCircles(xp, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(xp->d, xp->bggc, 0, LineDoubleDash, CapButt, JoinMiter);
    XSetLineAttributes(xp->d, xp->fggc, 0, LineDoubleDash, CapButt, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

Bool InitEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    int     size;
    int     rows;       /* Number of rows filled in current column	    */
    int     i;
    int     x, y;	    /* base of square to draw ellipse in	    */
    int     vsize, vsizeinc;
    int     dir;

    size = p->special;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    vsize = 1;
    vsizeinc = (size - 1) / (p->objects - 1);
    if (vsizeinc == 0) vsizeinc = 1;

    x = 0; y = 0;
    dir = 0;
    rows = 0;

    for (i = 0; i != p->objects; i++) {    
	arcs[i].x = x;
	arcs[i].y = y;
	if ((i & 1) ^ dir) {
	    /* Make vertical axis longer */
	    arcs[i].width = vsize;
	    arcs[i].height = size;
	} else {
	    /* Make horizontal axis longer */
	    arcs[i].width = size;
	    arcs[i].height = vsize;
	}
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;

	y += size + 1;
	rows++;
	if (y >= HEIGHT - size || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    y = 0;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		x = 0;
	    }
	}
	
	vsize += vsizeinc;
	if (vsize > size) {
	    vsize -= size;
	    dir = 1 - dir;
	}
    }
    return True;
}

Bool InitDashedEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    char dashes[2];

    (void)InitEllipses(xp, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(xp->d, xp->bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(xp->d, xp->fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

Bool InitDoubleDashedEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    char dashes[2];

    (void)InitEllipses(xp, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(xp->d, xp->bggc, 0, LineDoubleDash, CapButt, JoinMiter);
    XSetLineAttributes(xp->d, xp->fggc, 0, LineDoubleDash, CapButt, JoinMiter);
    dashes[0] = 3;   dashes[1] = 2;
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

void DoArcs(xp, p)
    XParms  xp;
    Parms   p;
{
    GC pgc;
    int i;

    pgc = xp->fggc;
    for (i = 0; i != p->reps; i++) {
        XDrawArcs(xp->d, xp->w, pgc, arcs, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void DoFilledArcs(xp, p)
    XParms  xp;
    Parms   p;
{
    GC pgc;
    int i;

    pgc = xp->fggc;
    for (i = 0; i != p->reps; i++) {
        XFillArcs(xp->d, xp->w, pgc, arcs, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void EndArcs(xp, p)
    XParms  xp;
    Parms   p;
{
    free(arcs);
}

