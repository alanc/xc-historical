/*
 * $XConsortium: miwideline.c,v 1.2 89/10/28 16:55:07 rws Exp $
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
miFillPolyHelper (pDrawable, pGC, pixel, y, overall_height,
		  left, right, left_count, right_count)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    unsigned long   pixel;
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
    int		oldPixel;
    int		xorg;

    left_height = 0;
    right_height = 0;
    
    pptInit = (DDXPointPtr) ALLOCATE_LOCAL (overall_height * sizeof(*ppt));
    if (!pptInit)
	return;
    pwidthInit = (int *) ALLOCATE_LOCAL (overall_height * sizeof(*pwidth));
    if (!pwidth)
    {
	DEALLOCATE_LOCAL (ppt);
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

	while (height-- > 0)
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
    (*pGC->ops->FillSpans) (pDrawable, pGC, ppt - pptInit, pptInit, pwidthInit, TRUE);
    DEALLOCATE_LOCAL (pwidthInit);
    DEALLOCATE_LOCAL (pptInit);
    if (pixel != oldPixel)
    {
	DoChangeGC (pGC, GCForeground, (XID *)&oldPixel, FALSE);
	ValidateGC (pDrawable, pGC);
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
    int	    t;

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
    edge->e = e;
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
    int	    idy, jdy;

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

miLineJoin (pDrawable, pGC, pixel, FillPoly, pLeft, pRight)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    void	    (*FillPoly)();
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
	miLineRoundCapJoin(pDrawable, pGC, pixel, pLeft->x, pLeft->y);
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
    (*FillPoly) (pDrawable, pGC, pixel, y, height, left, right, nleft, nright);
}

miLineRoundCapJoin (pDraw, pGC, pixel, xorg, yorg)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    unsigned long   pixel;
    int		    xorg, yorg;
{
    register int x, y, e, ex, slw;
    DDXPointPtr points;
    register DDXPointPtr pts;
    int *widths;
    register int *wids;
    unsigned long oldPixel;

    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * pGC->lineWidth);
    if (!points)
	return;
    widths = (int *)ALLOCATE_LOCAL(sizeof(int) * pGC->lineWidth);
    if (!widths)
    {
	DEALLOCATE_LOCAL(points);
	return;
    }
    pts = points;
    wids = widths;
    oldPixel = pGC->fgPixel;
    if (pixel != oldPixel)
    {
	DoChangeGC(pGC, GCForeground, (XID *)&pixel, FALSE);
	ValidateGC (pDraw, pGC);
    }
    if (pGC->miTranslate)
    {
	xorg += pDraw->x;
	yorg += pDraw->y;
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
    (*pGC->ops->FillSpans)(pDraw, pGC, pts - points, points, widths, FALSE);
    DEALLOCATE_LOCAL(widths);
    DEALLOCATE_LOCAL(points);
    if (pixel != oldPixel)
    {
	DoChangeGC(pGC, GCForeground, (XID *)&oldPixel, FALSE);
	ValidateGC (pDraw, pGC);
    }
}

miLineCapRound (pDraw, pGC, pixel, face, left)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    unsigned long   pixel;
    LineFacePtr	    face;
    Bool	    left;
{
    miLineRoundCapJoin(pDraw, pGC, pixel, face->x, face->y);
}

void
miWideSegment (pDrawable, pGC, pixel, FillPoly,
	       x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    void	    (*FillPoly)();
    int		    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    LineFacePtr	    leftFace, rightFace;
{
    double	l, L, r;
    double	xa, ya;
    double	projectXoff, projectYoff;
    double	k;
    int		x, y;
    int		e;
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
	lefts[0].e = 0;
	lefts[0].dx = 0;
	lefts[0].dy = lw;
	rights[0].height = lw;
	rights[0].x = x2 - 1;
	if (projectRight)
	    rights[0].x += (lw >> 1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = 0;
	rights[0].dx = 0;
	rights[0].dy = lw;
	(*FillPoly) (pDrawable, pGC, pixel, y1 - (lw >> 1), lw,
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
	lefts[0].e = 0;
	lefts[0].dx = dx;
	lefts[0].dy = dy;

	rights[0].height = bottomy - topy;
	rights[0].x = lefts[0].x + (lw-1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = 0;
	rights[0].dx = dx;
	rights[0].dy = dy;
	(*FillPoly) (pDrawable, pGC, pixel, topy, bottomy - topy, lefts, rights, 1, 1);
    }
    else
    {
    	l = ((double) lw) / 2.0;
    	L = hypot ((double) dx, (double) dy);

	if (projectLeft)
	{
	    projectXoff = dx / L * l;
	    projectYoff = dy / L * l;
	}
	else
	{
	    projectXoff = 0;
	    projectYoff = 0;
	}

	r = l / L;

	/* coord of upper bound at integral y */
	ya = -r * dx;
	xa = r * dy;

    	/* xa * dy - ya * dx */
	k = l * L;

	leftFace->xa = xa;
	leftFace->ya = ya;
	rightFace->xa = -xa;
	rightFace->ya = -ya;

	righty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
			    k, dx, dy, x1, y1, 0, &right);

	/* coord of lower bound at integral y */
	ya = -ya;
	xa = -xa;

	/* xa * dy - ya * dx */
	k = - k;

	lefty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
			   k, dx, dy, x1, y1, 1, &left);

	/* coord of top face at integral y */

	ya = -r * dx * signdx;
	xa = r * dy * signdx;

	k = 0;
	if (projectLeft)
	{
	    k = (xa - projectXoff) * dx + (ya - projectYoff) * dy;
	    xa -= projectXoff;
	    ya -= projectYoff;
	}

	topy = miPolyBuildEdge (xa, ya, k, -dy, dx, x1, y1, dx > 0, &top);

	/* coord of bottom face at integral y */
	ya = -r * dx * signdx;
	xa = r * dy * signdx;

	k = 0;
	if (projectRight)
	{
	    projectXoff = dx / L * l;
	    projectYoff = dy / L * l;
	    k = (xa + projectXoff) * dx + (ya + projectYoff) * dy;
	    xa += projectXoff;
	    ya += projectYoff;
	}

	bottomy = miPolyBuildEdge (xa, ya,
			     k, -dy, dx, x2, y2, dx < 0, &bottom);

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
	(*FillPoly) (pDrawable, pGC, pixel, topy, bottom.height + bottomy,
		     lefts, rights, 2, 2);
    }
}

miWideLine (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pPts;
{
    int		    x1, y1, x2, y2;
    void	    (*FillPoly)();
    unsigned long   pixel;
    Bool	    projectLeft, projectRight, doJoin;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    int		    first;

    if (npt == 0)
	return;
    FillPoly = pGC->ops->LineHelper;
    pixel = pGC->fgPixel;
    x2 = pPts->x;
    y2 = pPts->y;
    first = TRUE;
    doJoin = FALSE;
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
	projectLeft = FALSE;
	projectRight = FALSE;
	if (first)
	{
	    if (pGC->capStyle == CapProjecting)
		projectLeft = TRUE;
	}
	if (npt == 1)
	{
	    if (pGC->capStyle == CapProjecting)
		projectRight = TRUE;
	}
	miWideSegment (pDrawable, pGC, pixel, FillPoly, x1, y1, x2, y2,
		       projectLeft, projectRight, &leftFace, &rightFace);
	if (first)
	{
	    if (pGC->capStyle == CapRound)
		miLineCapRound (pDrawable, pGC, pixel, &leftFace, TRUE);
	}
	else
	{
	    miLineJoin (pDrawable, pGC, pixel, FillPoly, &leftFace,
		        &prevRightFace);
	}
	if (npt == 1 && pGC->capStyle == CapRound)
	    miLineCapRound (pDrawable, pGC, pixel, &rightFace, FALSE);
	prevRightFace = rightFace;
	first = FALSE;
    }
}

miMiter()
{
}

miNotMiter ()
{
}

miGetPts ()
{
}

miRoundCap()
{
}

miOneSegWide()
{
}

miExtendSegment ()
{
}
