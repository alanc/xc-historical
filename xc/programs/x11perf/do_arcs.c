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
static GC   pgc;

#define DegreesToX(degrees) (degrees * 64)

static Bool GenerateCircles(xp, p, partialArcs)
    XParms  xp;
    Parms   p;
    Bool    partialArcs;
{
    int     i;
    int     rows;       /* Number of rows filled in current column	    */
    int     x, y;       /* base of square to draw the circle in		    */
    int     xorg, yorg; /* Used to get from column to column or row to row  */
    int     size;
    int     half;
    int     startAngle, arcAngle;

    pgc = xp->fggc;

    size = p->special;
    half = (size + 19) / 20;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    x = xorg = half; y = yorg = half;
    rows = 0;
    startAngle = DegreesToX(0);
    arcAngle = DegreesToX(360);

    for (i = 0; i != p->objects; i++) {    
	arcs[i].x = x;
	arcs[i].y = y;
	arcs[i].width = size;
	arcs[i].height = size;
	arcs[i].angle1 = startAngle;
	arcs[i].angle2 = arcAngle;

	if (partialArcs) {
	    startAngle += DegreesToX(30);
	    if (startAngle >= DegreesToX(360)) startAngle -= DegreesToX(360);
	    arcAngle -= DegreesToX(20);
	    if (arcAngle <= DegreesToX(0)) arcAngle += DegreesToX(360);
	}

	y += size + 1;
	rows++;
	if (y >= HEIGHT - size  - half || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		yorg++;
		if (yorg >= size + half || yorg >= HEIGHT - size - half) {
		    yorg = half;
		    xorg++;
		    if (xorg >= size + half || xorg >= WIDTH - size - half) {
			xorg = half;
		    }
		}
		x = xorg;
	    }
	    y = yorg;
	}
    }
    return True;
}

Bool InitCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    return GenerateCircles(xp, p, False);
}

Bool InitPartCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    return GenerateCircles(xp, p, True);
}


Bool InitChordPartCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    (void) GenerateCircles(xp, p, True);
    XSetArcMode(xp->d, xp->bggc, ArcChord);
    XSetArcMode(xp->d, xp->fggc, ArcChord);
    return True;
}


Bool InitSlicePartCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    (void) GenerateCircles(xp, p, True);
    XSetArcMode(xp->d, xp->bggc, ArcPieSlice);
    XSetArcMode(xp->d, xp->fggc, ArcPieSlice);
    return True;
}


Bool InitWideCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    int size;

    (void)InitCircles(xp, p);

    size = p->special;
    XSetLineAttributes(xp->d, xp->bggc, (int) ((size + 9) / 10),
	LineSolid, CapRound, JoinRound);
    XSetLineAttributes(xp->d, xp->fggc, (int) ((size + 9) / 10),
	LineSolid, CapRound, JoinRound);

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

Bool InitWideDashedCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    int		size;
    XGCValues   gcv;
    char	dashes[2];

    (void)InitWideCircles(xp, p);
    size = p->special;
    size = (size + 9) / 10;

    /* Modify GCs to draw dashed */
    dashes[0] = 2*size;   dashes[1] = 2*size;
    gcv.line_style = LineOnOffDash;
    XChangeGC(xp->d, xp->fggc, GCLineStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCLineStyle, &gcv);
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

Bool InitWideDoubleDashedCircles(xp, p)
    XParms  xp;
    Parms   p;
{
    int		size;
    XGCValues   gcv;
    char	dashes[2];

    (void)InitWideCircles(xp, p);
    size = p->special;
    size = (size + 9) / 10;

    /* Modify GCs to draw dashed */
    dashes[0] = 2*size;   dashes[1] = 2*size;
    gcv.line_style = LineDoubleDash;
    XChangeGC(xp->d, xp->fggc, GCLineStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCLineStyle, &gcv);
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

static Bool GenerateEllipses(xp, p, partialArcs)
    XParms  xp;
    Parms   p;
    Bool    partialArcs;
{
    int     size;
    int     half;
    int     rows;       /* Number of rows filled in current column	    */
    int     i;
    int     x, y;	    /* base of square to draw ellipse in	    */
    int     vsize, vsizeinc;
    int     dir;
    int     startAngle, arcAngle;

    pgc = xp->fggc;

    size = p->special;
    half = (size + 19) / 20;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    vsize = 1;
    vsizeinc = (size - 1) / (p->objects - 1);
    if (vsizeinc == 0) vsizeinc = 1;

    x = half; y = half;
    dir = 0;
    rows = 0;
    startAngle = DegreesToX(0);
    arcAngle = DegreesToX(360);

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
	arcs[i].angle1 = startAngle;
	arcs[i].angle2 = arcAngle;

	if (partialArcs) {
	    startAngle += DegreesToX(30);
	    if (startAngle >= DegreesToX(360)) startAngle -= DegreesToX(360);
	    arcAngle -= DegreesToX(20);
	    if (arcAngle <= DegreesToX(0)) arcAngle += DegreesToX(360);
	}

	y += size + 1;
	rows++;
	if (y >= HEIGHT - size - half || rows == MAXROWS) {
	    /* Go to next column */
	    rows = 0;
	    y = half;
	    x += size + 1;
	    if (x >= WIDTH - size - half) {
		x = half;
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

Bool InitEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    return GenerateEllipses(xp, p, False);
}


Bool InitPartEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    return GenerateEllipses(xp, p, True);
}


Bool InitChordPartEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    (void) GenerateEllipses(xp, p, True);
    XSetArcMode(xp->d, xp->bggc, ArcChord);
    XSetArcMode(xp->d, xp->fggc, ArcChord);
    return True;
}


Bool InitSlicePartEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    (void) GenerateEllipses(xp, p, True);
    XSetArcMode(xp->d, xp->bggc, ArcPieSlice);
    XSetArcMode(xp->d, xp->fggc, ArcPieSlice);
    return True;
}


Bool InitWideEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    int size;

    (void)InitEllipses(xp, p);

    size = p->special;
    XSetLineAttributes(xp->d, xp->bggc, (int) ((size + 9) / 10),
	LineSolid, CapRound, JoinRound);
    XSetLineAttributes(xp->d, xp->fggc, (int) ((size + 9) / 10),
	LineSolid, CapRound, JoinRound);

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

Bool InitWideDashedEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    int		size;
    XGCValues   gcv;
    char	dashes[2];

    (void)InitWideEllipses(xp, p);
    size = p->special;
    size = (size + 9) / 10;

    /* Modify GCs to draw dashed */
    dashes[0] = 2*size;   dashes[1] = 2*size;
    gcv.line_style = LineOnOffDash;
    XChangeGC(xp->d, xp->fggc, GCLineStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCLineStyle, &gcv);
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

Bool InitWideDoubleDashedEllipses(xp, p)
    XParms  xp;
    Parms   p;
{
    int		size;
    XGCValues   gcv;
    char	dashes[2];

    (void)InitWideEllipses(xp, p);
    size = p->special;
    size = (size + 9) / 10;

    /* Modify GCs to draw dashed */
    dashes[0] = 2*size;   dashes[1] = 2*size;
    gcv.line_style = LineDoubleDash;
    XChangeGC(xp->d, xp->fggc, GCLineStyle, &gcv);
    XChangeGC(xp->d, xp->bggc, GCLineStyle, &gcv);
    XSetDashes(xp->d, xp->fggc, 0, dashes, 2);
    XSetDashes(xp->d, xp->bggc, 0, dashes, 2);
    return True;
}

void DoArcs(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

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
    int i;

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

