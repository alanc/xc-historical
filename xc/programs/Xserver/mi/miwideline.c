/*
 * $XConsortium: miwideline.c,v 1.13 89/11/02 12:04:09 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Mostly integer wideline code.  Uses a technique similar to
 * bresenham zero-width lines, except walks an X edge
 */

#include <stdio.h>
#include <math.h>
#include "X.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "miscstruct.h"
#include "miwideline.h"

#ifdef ICEILTEMPDECL
ICEILTEMPDECL
#endif

/*
 * spans-based polygon filler
 */

void
miFillPolyHelper (pDrawable, pGC, pixel, spanData, y, overall_height,
		  left, right, left_count, right_count)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    unsigned long   pixel;
    SpanDataPtr	spanData;
    int		y;			/* start y coordinate */
    int		overall_height;		/* height of entire segment */
    PolyEdgePtr	left, right;
    int		left_count, right_count;
{
    int	left_x;
    int	left_stepx;
    int	left_signdx;
    int	left_e, left_dy, left_dx;

    int	right_x;
    int	right_stepx;
    int	right_signdx;
    int	right_e, right_dy, right_dx;

    int	height;
    int	left_height, right_height;

    DDXPointPtr	ppt, pptInit;
    int		*pwidth, *pwidthInit;
    unsigned long oldPixel;
    int		xorg;
    Spans	spanRec;

    left_height = 0;
    right_height = 0;
    
    if (!spanData)
    {
    	pptInit = (DDXPointPtr) ALLOCATE_LOCAL (overall_height * sizeof(*ppt));
    	if (!pptInit)
	    return;
    	pwidthInit = (int *) ALLOCATE_LOCAL (overall_height * sizeof(*pwidth));
    	if (!pwidthInit)
    	{
	    DEALLOCATE_LOCAL (pptInit);
	    return;
    	}
	ppt = pptInit;
	pwidth = pwidthInit;
    	oldPixel = pGC->fgPixel;
    	if (pixel != oldPixel)
    	{
    	    DoChangeGC (pGC, GCForeground, (XID *)&pixel, FALSE);
    	    ValidateGC (pDrawable, pGC);
    	}
    }
    else
    {
	spanRec.points = (DDXPointPtr) xalloc (overall_height * sizeof (*ppt));
	if (!spanRec.points)
	    return;
	spanRec.widths = (int *) xalloc (overall_height * sizeof (int));
	if (!spanRec.widths)
	{
	    xfree (spanRec.points);
	    return;
	}
	ppt = spanRec.points;
	pwidth = spanRec.widths;
    }

    if (pGC->miTranslate)
    {
	y += pDrawable->y;
	xorg = pDrawable->x;
    }
    while ((left_count || left_height) &&
	   (right_count || right_height))
    {
	MIPOLYRELOADLEFT
	MIPOLYRELOADRIGHT

	height = left_height;
	if (height > right_height)
	    height = right_height;

	left_height -= height;
	right_height -= height;

	while (--height >= 0)
	{
	    if (right_x >= left_x)
	    {
		ppt->y = y;
		ppt->x = left_x + xorg;
		ppt++;
		*pwidth++ = right_x - left_x + 1;
	    }
    	    y++;
    	
	    MIPOLYSTEPLEFT

	    MIPOLYSTEPRIGHT
	}
    }
    if (!spanData)
    {
    	(*pGC->ops->FillSpans) (pDrawable, pGC, ppt - pptInit, pptInit, pwidthInit, TRUE);
    	DEALLOCATE_LOCAL (pwidthInit);
    	DEALLOCATE_LOCAL (pptInit);
    	if (pixel != oldPixel)
    	{
	    DoChangeGC (pGC, GCForeground, (XID *)&oldPixel, FALSE);
	    ValidateGC (pDrawable, pGC);
    	}
    }
    else
    {
	SpanGroup   *group;

	spanRec.count = ppt - spanRec.points;
	if (pixel == pGC->fgPixel)
	    group = &spanData->fgGroup;
	else
	    group = &spanData->bgGroup;
	miAppendSpans (group, &spanRec);
    }
}

int
miPolyBuildEdge (x0, y0, k, dx, dy, xi, yi, left, edge)
    double	x0, y0;
    double	k;  /* x0 * dy - y0 * dx */
    int		dx, dy;
    int		xi, yi;
    int		left;
    PolyEdgePtr	edge;
{
    int	    x, y, e;
    int	    signdx;
    int	    xady;

    if (dy < 0)
    {
	dy = -dy;
	dx = -dx;
	k = -k;
    }
    signdx = 1;
    if (dx < 0)
	signdx = -1;

    y = ICEIL (y0);
    xady = ICEIL (k) + y * dx;
    if (left)
    {
	if (xady <= 0)
	    x = xady / dy;
	else
	    x = (xady - 1) / dy + 1;
	if (dx < 0)
	    e = x * dy - xady;
	else
	    e = xady - x * dy - 1;
    }
    else
    {
	if (xady <= 0)
	    x = xady / dy - 1;
	else
	    x = (xady - 1) / dy;
	if (dx < 0)
	    e = x * dy - xady - 1;
	else
	    e = xady - x * dy;
    }

    if (e < 0)
	e += dy;

    edge->stepx = dx / dy;
    if ((edge->signdx = signdx) == 1)
	edge->dx = dx % dy;
    else
	edge->dx = (-dx) % dy;
    edge->dy = dy;
    edge->x = x + xi;
    edge->e = e - dy;
    return y + yi;
}

#define StepAround(v, incr, max) (((v) + (incr) < 0) ? (max - 1) : ((v) + (incr) == max) ? 0 : ((v) + (incr)))

int
miPolyBuildPoly (vertices, slopes, count, xi, yi, left, right, pnleft, pnright, h)
    PolyVertexPtr   vertices;
    PolySlopePtr    slopes;
    int		    count;
    int		    xi, yi;
    PolyEdgePtr	    left, right;
    int		    *pnleft, *pnright;
    int		    *h;
{
    int	    top, bottom;
    double  miny, maxy;
    int	    i, j;
    int	    clockwise;
    int	    slopeoff;
    int	    s;
    int	    nleft, nright;
    int	    y, lasty, bottomy, topy;

    /* find the top of the polygon */
    maxy = miny = vertices[0].y;
    bottom = top = 0;
    for (i = 1; i < count; i++)
    {
	if (vertices[i].y < miny)
	{
	    top = i;
	    miny = vertices[i].y;
	}
	if (vertices[i].y >= maxy)
	{
	    bottom = i;
	    maxy = vertices[i].y;
	}
    }
    clockwise = 1;
    slopeoff = 0;

    i = top;
    j = StepAround (top, -1, count);

    if (slopes[j].dy * slopes[i].dx > slopes[i].dy * slopes[j].dx)
    {
	clockwise = -1;
	slopeoff = -1;
    }

    bottomy = ICEIL (maxy) + yi;

    nright = 0;

    s = StepAround (top, slopeoff, count);
    i = top;
    while (i != bottom)
    {
	if (slopes[s].dy != 0)
	{
	    y = miPolyBuildEdge (vertices[i].x, vertices[i].y,
 		       vertices[i].x * slopes[s].dy -
			vertices[i].y * slopes[s].dx,
		       slopes[s].dx, slopes[s].dy,
		       xi, yi, 0,
		       &right[nright]);
	    if (nright != 0)
	    	right[nright-1].height = y - lasty;
	    else
	    	topy = y;
	    nright++;
	    lasty = y;
	}

	i = StepAround (i, clockwise, count);
	s = StepAround (s, clockwise, count);
    }
    if (nright != 0)
	right[nright-1].height = bottomy - lasty;

    if (slopeoff == 0)
	slopeoff = -1;
    else
	slopeoff = 0;

    nleft = 0;
    s = StepAround (top, slopeoff, count);
    i = top;
    while (i != bottom)
    {
	if (slopes[s].dy != 0)
	{
	    y = miPolyBuildEdge (vertices[i].x, vertices[i].y,
 		       vertices[i].x * slopes[s].dy -
			vertices[i].y * slopes[s].dx,
		       	   slopes[s].dx,  slopes[s].dy, xi, yi, 1,
		       	   &left[nleft]);
    
	    if (nleft != 0)
	    	left[nleft-1].height = y - lasty;
	    nleft++;
	    lasty = y;
	}
	i = StepAround (i, -clockwise, count);
	s = StepAround (s, -clockwise, count);
    }
    if (nleft != 0)
	left[nleft-1].height = bottomy - lasty;
    *pnleft = nleft;
    *pnright = nright;
    *h = bottomy - topy;
    return topy;
}

miLineJoin (pDrawable, pGC, pixel, spanData, pLeft, pRight)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    SpanDataPtr	    spanData;
    LineFacePtr	    pLeft, pRight;
{
    double	    mx, my;
    int		    denom;
    PolyVertexRec   vertices[4];
    PolySlopeRec    slopes[4];
    int		    edgecount;
    PolyEdgeRec	    left[4], right[4];
    int		    nleft, nright;
    int		    y, height;
    int		    swapslopes;
    int		    joinStyle = pGC->joinStyle;
    int		    lw = pGC->lineWidth;

    denom = - pLeft->dx * pRight->dy + pRight->dx * pLeft->dy;
    if (denom == 0)
	return;	/* no join to draw */

    swapslopes = 0;
    if (denom > 0)
    {
	pLeft->xa = -pLeft->xa;
	pLeft->ya = -pLeft->ya;
	pLeft->dx = -pLeft->dx;
	pLeft->dy = -pLeft->dy;
    }
    else
    {
	swapslopes = 1;
	pRight->xa = -pRight->xa;
	pRight->ya = -pRight->ya;
	pRight->dx = -pRight->dx;
	pRight->dy = -pRight->dy;
    }

    vertices[0].x = pRight->xa;
    vertices[0].y = pRight->ya;
    slopes[0].dx = -pRight->dy;
    slopes[0].dy =  pRight->dx;

    vertices[1].x = 0;
    vertices[1].y = 0;
    slopes[1].dx =  pLeft->dy;
    slopes[1].dy = -pLeft->dx;

    vertices[2].x = pLeft->xa;
    vertices[2].y = pLeft->ya;

    if (joinStyle == JoinMiter)
    {
	slopes[2].dx = -pLeft->dx;
	slopes[2].dy = -pLeft->dy;

    	my = (pLeft->dy  * (pRight->xa * pRight->dy - pRight->ya * pRight->dx) -
              pRight->dy * (pLeft->xa  * pLeft->dy  - pLeft->ya  * pLeft->dx )) /
	      (double) denom;
    	if (pLeft->dy != 0)
    	{
	    mx = pLeft->xa + (my - pLeft->ya) *
			    (double) pLeft->dx / (double) pLeft->dy;
    	}
    	else
    	{
	    mx = pRight->xa + (my - pRight->ya) *
			    (double) pRight->dx / (double) pRight->dy;
    	}
	/* check miter limit */
	if ((mx * mx + my * my) * 4 > SQSECANT * lw * lw)
	    joinStyle = JoinBevel;
    }

    switch (joinStyle)
    {
    case JoinRound:
	miLineArc(pDrawable, pGC, pixel, spanData, pLeft->x, pLeft->y,
		  (double)0.0, (double)0.0, TRUE);
	return;
    case JoinMiter:
	slopes[2].dx = pLeft->dx;
	slopes[2].dy = pLeft->dy;
	if (swapslopes)
	{
	    slopes[2].dx = -slopes[2].dx;
	    slopes[2].dy = -slopes[2].dy;
	}
	vertices[3].x = mx;
	vertices[3].y = my;
	slopes[3].dx = pRight->dx;
	slopes[3].dy = pRight->dy;
	if (swapslopes)
	{
	    slopes[3].dx = -slopes[3].dx;
	    slopes[3].dy = -slopes[3].dy;
	}
	edgecount = 4;
	break;
    case JoinBevel:
	{
	    double	scale, dx, dy, adx, ady;
    
	    adx = dx = pRight->xa - pLeft->xa;
	    ady = dy = pRight->ya - pLeft->ya;
	    if (adx < 0)
		adx = -adx;
	    if (ady < 0)
		ady = -ady;
	    scale = ady;
	    if (adx > ady)
	    	scale = adx;
	    slopes[2].dx = (dx * 65536) / scale;
	    slopes[2].dy = (dy * 65536) / scale;
	    edgecount = 3;
	}
	break;
    }

    y = miPolyBuildPoly (vertices, slopes, edgecount, pLeft->x, pLeft->y,
		   left, right, &nleft, &nright, &height);
    miFillPolyHelper (pDrawable, pGC, pixel, spanData, y, height, left, right, nleft, nright);
}

int
miLineArcI (pDraw, pGC, xorg, yorg, points, widths)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    int		    xorg, yorg;
    DDXPointPtr	    points;
    int		    *widths;
{
    register int x, y, e, ex, slw;
    register DDXPointPtr pts;
    register int *wids;

    pts = points;
    wids = widths;
    if (pGC->miTranslate)
    {
	xorg += pDraw->x;
	yorg += pDraw->y;
    }
    if (pGC->lineWidth == 1)
    {
	pts->x = xorg;
	pts->y = yorg;
	*wids = 1;
	return 1;
    }
    y = (pGC->lineWidth >> 1) + 1;
    if (pGC->lineWidth & 1)
	e = - ((y << 2) + 3);
    else
	e = - (y << 3);
    ex = -4;
    x = 0;
    while (y)
    {
	e += (y << 3) - 4;
	while (e >= 0)
	{
	    x++;
	    e += (ex = -((x << 3) + 4));
	}
	y--;
	slw = (x << 1) + 1;
	if ((e == ex) && (slw > 1))
	    slw--;
	pts->x = xorg - x;
	pts->y = yorg - y;
	pts++;
	*wids++ = slw;
	if ((y != 0) && ((slw > 1) || (e != ex)))
	{
	    pts->x = xorg - x;
	    pts->y = yorg + y;
	    pts++;
	    *wids++ = slw;
	}
    }
    return (pts - points);
}

int
miLineArcD (pDraw, pGC, xorg, yorg, points, widths)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    double	    xorg, yorg;
    DDXPointPtr	    points;
    int		    *widths;
{
    register DDXPointPtr pts;
    register int *wids;
    double radius, x0, y0, el, er, yk, xlk, xrk, k;
    int xbase, ybase, y, boty, xl, xr;

    pts = points;
    wids = widths;
    xbase = floor(xorg);
    x0 = xorg - xbase;
    ybase = ICEIL (yorg);
    y0 = yorg - ybase;
    if (pGC->miTranslate)
    {
	xbase += pDraw->x;
	ybase += pDraw->y;
    }
    xlk = x0 + x0 + 1.0;
    xrk = x0 + x0 - 1.0;
    yk = y0 + y0 - 1.0;
    radius = ((double)pGC->lineWidth) / 2.0;
    y = floor(radius - y0 + 1.0);
    ybase -= y;
    el = radius * radius - ((y + y0) * (y + y0)) - (x0 * x0);
    er = el + xrk;
    xl = 1;
    xr = 0;
    if (x0 < 0.5)
    {
	xl = 0;
	el -= xlk;
    }
    boty = (y0 < -0.5) ? 1 : 0;
    while (y > boty)
    {
	k = (y << 1) + yk;
	er += k;
	while (er > 0.0)
	{
	    xr++;
	    er += xrk - (xr << 1);
	}
	el += k;
	while (el >= 0.0)
	{
	    xl--;
	    el += (xl << 1) - xlk;
	}
	y--;
	ybase++;
	if (xr >= xl)
	{
	    pts->x = xbase + xl;
	    pts->y = ybase;
	    pts++;
	    *wids++ = xr - xl + 1;
	}
    }
    er = xrk - (xr << 1) - er;
    el = (xl << 1) - xlk - el;
    boty = floor(-y0 - radius + 1.0);
    while (y > boty)
    {
	k = (y << 1) + yk;
	er -= k;
	while ((er >= 0.0) && (xr >= 0))
	{
	    xr--;
	    er += xrk - (xr << 1);
	}
	el -= k;
	while ((el > 0.0) && (xl <= 0))
	{
	    xl++;
	    el += (xl << 1) - xlk;
	}
	y--;
	ybase++;
	if (xr >= xl)
	{
	    pts->x = xbase + xl;
	    pts->y = ybase;
	    pts++;
	    *wids++ = xr - xl + 1;
	}
    }
    return (pts - points);
}

miLineArc (pDraw, pGC, pixel, spanData, xorgi, yorgi, xorg, yorg, isInteger)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    unsigned long   pixel;
    SpanDataPtr	    spanData;
    int		    xorgi, yorgi;
    double	    xorg, yorg;
    Bool	    isInteger;
{
    DDXPointPtr points;
    int *widths;
    unsigned long oldPixel;
    Spans spanRec;
    int n;

    if (!spanData)
    {
    	points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * pGC->lineWidth);
    	if (!points)
	    return;
    	widths = (int *)ALLOCATE_LOCAL(sizeof(int) * pGC->lineWidth);
    	if (!widths)
    	{
	    DEALLOCATE_LOCAL(points);
	    return;
    	}
    	oldPixel = pGC->fgPixel;
    	if (pixel != oldPixel)
    	{
	    DoChangeGC(pGC, GCForeground, (XID *)&pixel, FALSE);
	    ValidateGC (pDraw, pGC);
    	}
    }
    else
    {
	points = (DDXPointPtr) xalloc (pGC->lineWidth * sizeof (DDXPointRec));
	if (!points)
	    return;
	widths = (int *) xalloc (pGC->lineWidth * sizeof (int));
	if (!widths)
	{
	    xfree (points);
	    return;
	}
	spanRec.points = points;
	spanRec.widths = widths;
    }
    if (isInteger)
	n = miLineArcI(pDraw, pGC, xorgi, yorgi, points, widths);
    else
	n = miLineArcD(pDraw, pGC, xorg, yorg, points, widths);

    if (!spanData)
    {
    	(*pGC->ops->FillSpans)(pDraw, pGC, n, points, widths, FALSE);
    	DEALLOCATE_LOCAL(widths);
    	DEALLOCATE_LOCAL(points);
    	if (pixel != oldPixel)
    	{
	    DoChangeGC(pGC, GCForeground, (XID *)&oldPixel, FALSE);
	    ValidateGC (pDraw, pGC);
    	}
    }
    else
    {
	SpanGroup   *group;

	spanRec.count = n;
	if (pixel == pGC->fgPixel)
	    group = &spanData->fgGroup;
	else
	    group = &spanData->bgGroup;
	miAppendSpans (group, &spanRec);
    }
}

void
miWideSegment (pDrawable, pGC, pixel, spanData,
	       x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    SpanDataPtr	    spanData;
    int		    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    LineFacePtr	    leftFace, rightFace;
{
    double	l, L, r;
    double	xa, ya;
    double	projectXoff, projectYoff;
    double	k;
    int		x, y;
    int		dx, dy;
    PolyEdgeRec	left, right;
    PolyEdgeRec	top, bottom;
    int		lefty, righty, topy, bottomy;
    int		signdx;
    PolyEdgeRec	lefts[2], rights[2];
    LineFacePtr	tface;
    int		lw = pGC->lineWidth;

    /* draw top-to-bottom always */
    if (y2 < y1 || y2 == y1 && x2 < x1)
    {
	x = x1;
	x1 = x2;
	x2 = x;

	y = y1;
	y1 = y2;
	y2 = y;

	x = projectLeft;
	projectLeft = projectRight;
	projectRight = x;

	tface = leftFace;
	leftFace = rightFace;
	rightFace = tface;
    }

    dy = y2 - y1;
    signdx = 1;
    dx = x2 - x1;
    if (dx < 0)
	signdx = -1;

    leftFace->x = x1;
    leftFace->y = y1;
    leftFace->dx = dx;
    leftFace->dy = dy;

    rightFace->x = x2;
    rightFace->y = y2;
    rightFace->dx = -dx;
    rightFace->dy = -dy;

    if (dy == 0)
    {
	rightFace->xa = 0;
	rightFace->ya = (double) lw / 2.0;
	leftFace->xa = 0;
	leftFace->ya = -rightFace->ya;
	lefts[0].height = lw;
	lefts[0].x = x1;
	if (projectLeft)
	    lefts[0].x -= (lw >> 1);
	lefts[0].stepx = 0;
	lefts[0].signdx = 1;
	lefts[0].e = -lw;
	lefts[0].dx = 0;
	lefts[0].dy = lw;
	rights[0].height = lw;
	rights[0].x = x2 - 1;
	if (projectRight)
	    rights[0].x += (lw >> 1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = -lw;
	rights[0].dx = 0;
	rights[0].dy = lw;
	miFillPolyHelper (pDrawable, pGC, pixel, spanData, y1 - (lw >> 1), lw,
		     lefts, rights, 1, 1);
    }
    else if (dx == 0)
    {
	leftFace->xa =  (double) lw / 2.0;
	leftFace->ya = 0;
	rightFace->xa = -leftFace->xa;;
	rightFace->ya = 0;
	topy = y1;
	bottomy = y1 + dy;
	if (projectLeft)
	    topy -= lw / 2;
	if (projectRight)
	    bottomy += lw/2;
	lefts[0].height = bottomy - topy;
	lefts[0].x = x1 - (lw >> 1);
	lefts[0].stepx = 0;
	lefts[0].signdx = 1;
	lefts[0].e = -dy;
	lefts[0].dx = dx;
	lefts[0].dy = dy;

	rights[0].height = bottomy - topy;
	rights[0].x = lefts[0].x + (lw-1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = -dy;
	rights[0].dx = dx;
	rights[0].dy = dy;
	miFillPolyHelper (pDrawable, pGC, pixel, spanData, topy, bottomy - topy, lefts, rights, 1, 1);
    }
    else
    {
    	l = ((double) lw) / 2.0;
    	L = hypot ((double) dx, (double) dy);

	r = l / L;

	/* coord of upper bound at integral y */
	ya = -r * dx;
	xa = r * dy;

	if (projectLeft | projectRight)
	{
	    projectXoff = -ya;
	    projectYoff = xa;
	}

    	/* xa * dy - ya * dx */
	k = l * L;

	leftFace->xa = xa;
	leftFace->ya = ya;
	rightFace->xa = -xa;
	rightFace->ya = -ya;

	if (projectLeft)
	    righty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				      k, dx, dy, x1, y1, 0, &right);
	else
	    righty = miPolyBuildEdge (xa, ya,
				      k, dx, dy, x1, y1, 0, &right);

	/* coord of lower bound at integral y */
	ya = -ya;
	xa = -xa;

	/* xa * dy - ya * dx */
	k = - k;

	if (projectLeft)
	    lefty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				     k, dx, dy, x1, y1, 1, &left);
	else
	    lefty = miPolyBuildEdge (xa, ya,
				     k, dx, dy, x1, y1, 1, &left);

	/* coord of top face at integral y */

	if (signdx > 0)
	{
	    ya = -ya;
	    xa = -xa;
	}

	if (projectLeft)
	{
	    double xap = xa - projectXoff;
	    double yap = ya - projectYoff;
	    topy = miPolyBuildEdge (xap, yap, xap * dx + yap * dy,
				    -dy, dx, x1, y1, dx > 0, &top);
	}
	else
	    topy = miPolyBuildEdge (xa, ya, 0.0, -dy, dx, x1, y1, dx > 0, &top);

	/* coord of bottom face at integral y */

	if (projectRight)
	{
	    double xap = xa + projectXoff;
	    double yap = ya + projectYoff;
	    bottomy = miPolyBuildEdge (xap, yap, xap * dx + yap * dy,
				       -dy, dx, x2, y2, dx < 0, &bottom);
	}
	else
	    bottomy = miPolyBuildEdge (xa, ya,
				       0.0, -dy, dx, x2, y2, dx < 0, &bottom);

	if (dx < 0)
	{
	    left.height = bottomy - lefty;
	    right.height = bottomy - lefty;
	    top.height = righty - topy;
	    bottom.height = righty - topy;
	    lefts[0] = left;
	    lefts[1] = bottom;
	    rights[0] = top;
	    rights[1] = right;
	}
	else
	{
	    left.height = bottomy - righty;
	    right.height = bottomy - righty;
	    top.height = lefty - topy;
	    bottom.height = lefty - topy;
	    lefts[0] = top;
	    lefts[1] = left;
	    rights[0] = right;
	    rights[1] = bottom;
	}
	miFillPolyHelper (pDrawable, pGC, pixel, spanData, topy,
		     bottom.height + bottomy - topy, lefts, rights, 2, 2);
    }
}

static SpanDataPtr
miSetupSpanData (pGC, spanData)
    GCPtr	pGC;
    SpanDataPtr	spanData;
{
    switch(pGC->alu)
    {
    case GXclear:		/* 0 */
    case GXcopy:		/* src */
    case GXcopyInverted:	/* NOT src */
    case GXset:		/* 1 */
    case GXand:		/* src AND dst */
    case GXnoop:		/* dst */
    case GXor:		/* src OR dst */
    case GXorInverted:	/* NOT src OR dst */
    case GXandInverted:	/* NOT src AND dst */
	spanData = (SpanDataPtr) NULL;
	break;
    case GXandReverse:	/* src AND NOT dst */
    case GXnor:		/* NOT src AND NOT dst */
    case GXequiv:		/* NOT src XOR dst */
    case GXinvert:		/* NOT dst */
    case GXorReverse:		/* src OR NOT dst */
    case GXnand:		/* NOT src OR NOT dst */
	if (pGC->lineStyle == LineDoubleDash)
	    miInitSpanGroup (&spanData->bgGroup);
	miInitSpanGroup (&spanData->fgGroup);
    }
    return spanData;
}

static void
miCleanupSpanData (pDrawable, pGC, spanData)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    SpanDataPtr	spanData;
{
    if (pGC->lineStyle == LineDoubleDash)
    {
	miFillUniqueSpanGroup (pDrawable, pGC, &spanData->bgGroup);
	miFreeSpanGroup (&spanData->bgGroup);
    }
    miFillUniqueSpanGroup (pDrawable, pGC, &spanData->fgGroup);
    miFreeSpanGroup (&spanData->fgGroup);
}

miWideLine (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pPts;
{
    int		    x1, y1, x2, y2;
    SpanDataRec	    spanDataRec;
    SpanDataPtr	    spanData;
    unsigned long   pixel;
    Bool	    projectLeft, projectRight;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    int		    first;
    Bool	    somethingDrawn = FALSE;

    if (npt == 0)
	return;

    spanData = miSetupSpanData (pGC, &spanDataRec);
    pixel = pGC->fgPixel;
    x2 = pPts->x;
    y2 = pPts->y;
    first = TRUE;
    projectLeft = pGC->capStyle == CapProjecting;
    projectRight = FALSE;
    while (--npt)
    {
	x1 = x2;
	y1 = y2;
	++pPts;
	x2 = pPts->x;
	y2 = pPts->y;
	if (mode == CoordModePrevious)
	{
	    x2 += x1;
	    y2 += y1;
	}
	if (x1 == x2 && y1 == y2)
	    continue;
	somethingDrawn = TRUE;
	if (npt == 1 && pGC->capStyle == CapProjecting)
	    projectRight = TRUE;
	miWideSegment (pDrawable, pGC, pixel, spanData, x1, y1, x2, y2,
		       projectLeft, projectRight, &leftFace, &rightFace);
	if (first)
	{
	    if (pGC->capStyle == CapRound)
		miLineArc (pDrawable, pGC, pixel, spanData,
			   leftFace.x, leftFace.y, (double)0.0, (double)0.0,
			   TRUE);
	}
	else
	{
	    miLineJoin (pDrawable, pGC, pixel, spanData, &leftFace,
		        &prevRightFace);
	}
	if (npt == 1 && pGC->capStyle == CapRound)
	    miLineArc (pDrawable, pGC, pixel, spanData,
		       rightFace.x, rightFace.y, (double)0.0, (double)0.0,
		       TRUE);
	prevRightFace = rightFace;
	first = FALSE;
	projectLeft = FALSE;
    }
    /* handle crock where all points are coincedent */
    if (!somethingDrawn)
    {
	projectLeft = pGC->capStyle == CapProjecting;
	miWideSegment (pDrawable, pGC, pixel, spanData,
		       x2, y2, x2, y2, projectLeft, projectLeft,
		       &leftFace, &rightFace);
	if (pGC->capStyle == CapRound)
	{
	    miLineArc (pDrawable, pGC, pixel, spanData,
		       leftFace.x, leftFace.y, (double)0.0, (double)0.0,
		       TRUE);
	    miLineArc (pDrawable, pGC, pixel, spanData,
		       rightFace.x, rightFace.y, (double)0.0, (double)0.0,
		       TRUE);
	}
    }
    if (spanData)
	miCleanupSpanData (pDrawable, pGC, spanData);
}

#define V_TOP	    0
#define V_RIGHT	    1
#define V_BOTTOM    2
#define V_LEFT	    3

miWideDashSegment (pDrawable, pGC, spanData, pDashOffset, pDashIndex,
	   x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    *pDashOffset, *pDashIndex;
    SpanDataPtr	    spanData;
    int		    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    LineFacePtr	    leftFace, rightFace;
{
    int		    dashIndex, dashRemain;
    unsigned char   *pDash;
    double	    L, l;
    PolyVertexRec   vertices[4];
    PolyVertexRec   saveRight, saveBottom;
    PolySlopeRec    slopes[4];
    PolyEdgeRec	    left[2], right[2];
    int		    nleft, nright;
    int		    h;
    int		    y;
    int		    dy, dx;
    unsigned long   pixel;
    double	    LRemain;
    double	    r;
    double	    rdx, rdy;
    double	    dashDx, dashDy;
    Bool	    first = TRUE;
    double	    lcenterx, lcentery, rcenterx, rcentery;
    
    dx = x2 - x1;
    dy = y2 - y1;
    dashIndex = *pDashIndex;
    pDash = pGC->dash;
    dashRemain = pDash[dashIndex] - *pDashOffset;

    l = ((double) pGC->lineWidth) / 2.0;
    L = hypot ((double) dx, (double) dy);
    if (L)
    {
	r = l / L;

	rdx = r * dx;
	rdy = r * dy;
    }
    else
    {
	rdx = l;
	rdy = 0;
    }
    LRemain = L;
    /* All position comments are relative to a line with dx and dy > 0,
     * but the code does not depend on this */
    /* top */
    slopes[V_TOP].dx = dx;
    slopes[V_TOP].dy = dy;
    /* right */
    slopes[V_RIGHT].dx = -dy;
    slopes[V_RIGHT].dy = dx;
    /* bottom */
    slopes[V_BOTTOM].dx = -dx;
    slopes[V_BOTTOM].dy = -dy;
    /* left */
    slopes[V_LEFT].dx = dy;
    slopes[V_LEFT].dy = -dx;

    leftFace->x = x1;
    leftFace->y = y1;
    leftFace->dx = dx;
    leftFace->dy = dy;

    rightFace->x = x2;	/* for round dash caps */
    rightFace->y = y2;
    rightFace->dx = -dx;
    rightFace->dy = -dy;

    /* preload the start coordinates */
    vertices[V_RIGHT].x = vertices[V_TOP].x = rdy;
    vertices[V_RIGHT].y = vertices[V_TOP].y = -rdx;

    vertices[V_BOTTOM].x = vertices[V_LEFT].x = -rdy;
    vertices[V_BOTTOM].y = vertices[V_LEFT].y = rdx;

    if (projectLeft)
    {
	vertices[V_TOP].x -= rdx;
	vertices[V_TOP].y -= rdy;

	vertices[V_LEFT].x -= rdx;
	vertices[V_LEFT].y -= rdy;
    }

    lcenterx = x1;
    lcentery = y1;

    while (LRemain > dashRemain)
    {
	dashDx = (dashRemain * dx) / L;
	dashDy = (dashRemain * dy) / L;

	rcenterx = lcenterx + dashDx;
	rcentery = lcentery + dashDy;

	vertices[V_RIGHT].x += dashDx;
	vertices[V_RIGHT].y += dashDy;

	vertices[V_BOTTOM].x += dashDx;
	vertices[V_BOTTOM].y += dashDy;

	if (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1))
	{
	    if (pGC->lineStyle == LineOnOffDash &&
 	        pGC->capStyle == CapProjecting)
	    {
		saveRight = vertices[V_RIGHT];
		saveBottom = vertices[V_BOTTOM];
		
		if (!first)
		{
		    vertices[V_TOP].x -= rdx;
		    vertices[V_TOP].y -= rdy;
    
		    vertices[V_LEFT].x -= rdx;
		    vertices[V_LEFT].y -= rdy;
		}
		
		vertices[V_RIGHT].x += rdx;
		vertices[V_RIGHT].x += rdy;

		vertices[V_BOTTOM].x += rdx;
		vertices[V_BOTTOM].y += rdy;

	    }
	    y = miPolyBuildPoly (vertices, slopes, 4, x1, y1,
			     	 left, right, &nleft, &nright, &h);
	    pixel = (dashIndex & 1) ? pGC->bgPixel : pGC->fgPixel;
	    miFillPolyHelper (pDrawable, pGC, pixel, spanData, y, h, left, right, nleft, nright);

	    if (pGC->lineStyle == LineOnOffDash)
	    {
		switch (pGC->capStyle)
		{
		case CapProjecting:
		    vertices[V_BOTTOM] = saveBottom;
		    vertices[V_RIGHT] = saveRight;
		    break;
		case CapRound:
		    miLineArc (pDrawable, pGC, pixel, spanData, 0, 0,
			       lcenterx, lcentery, FALSE);
		    miLineArc (pDrawable, pGC, pixel, spanData, 0, 0,
			       rcenterx, rcentery, FALSE);
		    break;
		}
	    }
	}
	LRemain -= dashRemain;
	++dashIndex;
	if (dashIndex == pGC->numInDashList)
	    dashIndex = 0;
	dashRemain = pDash[dashIndex];

	lcenterx = rcenterx;
	lcentery = rcentery;

	vertices[V_TOP] = vertices[V_RIGHT];
	vertices[V_LEFT] = vertices[V_BOTTOM];
	first = FALSE;
    }

    if (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1))
    {
    	vertices[V_TOP].x -= dx;
    	vertices[V_TOP].y -= dy;

	vertices[V_LEFT].x -= dx;
	vertices[V_LEFT].y -= dy;

	vertices[V_RIGHT].x = rdy;
	vertices[V_RIGHT].y = -rdx;

	vertices[V_BOTTOM].x = -rdy;
	vertices[V_BOTTOM].y = rdx;
	
	if (projectRight)
	{
	    vertices[V_RIGHT].x += rdx;
	    vertices[V_RIGHT].y += rdy;
    
	    vertices[V_BOTTOM].x += rdx;
	    vertices[V_BOTTOM].y += rdy;
	}

	if (!first && pGC->lineStyle == LineOnOffDash &&
	    pGC->capStyle == CapProjecting)
	{
	    vertices[V_TOP].x -= rdx;
	    vertices[V_TOP].y -= rdy;

	    vertices[V_LEFT].x -= rdx;
	    vertices[V_LEFT].y -= rdy;
	}

	y = miPolyBuildPoly (vertices, slopes, 4, x2, y2,
			     left, right, &nleft, &nright, &h);

	pixel = (dashIndex & 1) ? pGC->bgPixel : pGC->fgPixel;
	miFillPolyHelper (pDrawable, pGC, pixel, spanData, y, h, left, right, nleft, nright);
	if (!first && pGC->lineStyle == LineOnOffDash &&
	    pGC->capStyle == CapRound)
	{
	    miLineArc (pDrawable, pGC, pixel, spanData, 0, 0,
		       lcenterx, lcentery, FALSE);
	}
    }
    dashRemain = ((double) dashRemain) - LRemain;
    if (dashRemain == 0)
    {
	dashIndex++;
	if (dashIndex == pGC->numInDashList)
	    dashIndex = 0;
	dashRemain = pDash[dashIndex];
    }

    leftFace->xa = rdy;
    leftFace->ya = -rdx;

    rightFace->x = x2;
    rightFace->y = y2;
    rightFace->xa = -rdy;
    rightFace->ya = rdx;

    *pDashIndex = dashIndex;
    *pDashOffset = pDash[dashIndex] - dashRemain;
}

miWideDash (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pPts;
{
    int		    x1, y1, x2, y2;
    unsigned long   pixel;
    Bool	    projectLeft, projectRight;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    int		    first;
    int		    dashIndex, dashOffset;
    SpanDataRec	    spanDataRec;
    SpanDataPtr	    spanData;

    if (npt == 0)
	return;
    spanData = miSetupSpanData (pGC, &spanDataRec);
    x2 = pPts->x;
    y2 = pPts->y;
    first = TRUE;
    projectLeft = pGC->capStyle == CapProjecting;
    projectRight = FALSE;
    dashIndex = 0;
    dashOffset = 0;
    miStepDash (pGC->dashOffset, &dashIndex,
	        pGC->dash, pGC->numInDashList, &dashOffset);
    while (--npt)
    {
	x1 = x2;
	y1 = y2;
	++pPts;
	x2 = pPts->x;
	y2 = pPts->y;
	if (mode == CoordModePrevious)
	{
	    x2 += x1;
	    y2 += y1;
	}
	if (x1 == x2 && y1 == y2)
	    continue;
	if (npt == 1 && pGC->capStyle == CapProjecting)
	    projectRight = TRUE;
	miWideDashSegment (pDrawable, pGC, spanData, &dashOffset, &dashIndex,
			   x1, y1, x2, y2,
			   projectLeft, projectRight, &leftFace, &rightFace);
	if (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1))
	{
	    pixel = (dashIndex & 1) ? pGC->bgPixel : pGC->fgPixel;
	    if (first)
	    {
	    	if (pGC->capStyle == CapRound)
		    miLineArc (pDrawable, pGC, pixel, spanData,
			       leftFace.x, leftFace.y,
			       (double)0.0, (double)0.0, TRUE);
	    }
	    else
	    {
	    	miLineJoin (pDrawable, pGC, pixel, spanData, &leftFace,
		            &prevRightFace);
	    }
	    if (npt == 1 && pGC->capStyle == CapRound)
	    	miLineArc (pDrawable, pGC, pixel, spanData,
			   rightFace.x, rightFace.y,
			   (double)0.0, (double)0.0, TRUE);
	}
	prevRightFace = rightFace;
	first = FALSE;
	projectLeft = FALSE;
    }
    if (spanData)
	miCleanupSpanData (pDrawable, pGC, spanData);
}

/* these are stubs to allow old ddx ValidateGCs to work without change */

void
miMiter()
{
}

void
miNotMiter()
{
}
