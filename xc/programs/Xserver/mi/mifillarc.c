/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* $XConsortium: mifillarc.c,v 5.3 89/10/22 15:38:22 rws Exp $ */

#include <math.h>
#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mifpoly.h"
#include "mi.h"
#include "mifillarc.h"

#define QUADRANT (90 * 64)
#define HALFCIRCLE (180 * 64)
#define QUADRANT3 (270 * 64)

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define Dsin(d)	sin((double)d*(M_PI/11520.0))
#define Dcos(d)	cos((double)d*(M_PI/11520.0))

void
miFillArcSetup(arc, info)
    register xArc *arc;
    register miFillArcRec *info;
{
    if (arc->width == arc->height)
    {
	/* (2x - 2xorg)^2 = d^2 - (2y - 2yorg)^2 */
	/* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
	info->ym = 8;
	info->xm = 8;
	info->y = arc->width >> 1;
	info->dy = arc->width & 1;
	info->yorg = arc->y + info->y;
	info->xorg = arc->x + info->y + info->dy;
	info->dx = 1 - info->dy;
	if (arc->width & 1)
	{
	    info->e = -1;
	    info->xk = 0;
	}
	else
	{
	    info->y++;
	    info->e = - (info->y << 3);
	    info->xk = 4;
	}
	info->ex = -info->xk;
	info->yk = info->xk;
    }
    else
    {
	/* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
	/* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
	info->ym = (arc->width * arc->width) << 3;
	info->xm = (arc->height * arc->height) << 3;
	info->y = arc->height >> 1;
	info->yorg = arc->y + info->y;
	info->dy = arc->height & 1;
	info->dx = arc->width & 1;
	info->xorg = arc->x + (arc->width >> 1) + info->dx;
	info->dx = 1 - info->dx;
	if (arc->height & 1)
	    info->yk = 0;
	else
	    info->yk = info->ym >> 1;
	if (arc->width & 1)
	{
	    info->xk = 0;
	    info->e = - (info->xm >> 3);
	}
	else
	{
	    info->y++;
	    info->xk = info->xm >> 1;
	    info->e = info->yk - (info->ym * info->y) - info->xk;
	}
	info->ex = -info->xk;
    }
}

static void
miGetArcEdge(arc, angle, edge, top, left)
    xArc *arc;
    int angle;
    miSliceEdgePtr edge;
    Bool top, left;
{
    int y, x2dy, signdx, signdy, xorg;
    double cx, sy, scale;

    if ((angle == 0) || (angle == HALFCIRCLE))
    {
	edge->x = left ? -65536 : 65536;
	edge->stepx = 0;
	edge->e = 0;
	edge->dx = -1;
	return;
    }
    xorg = arc->x + (arc->width >> 1);
    if ((angle == QUADRANT) || (angle == QUADRANT3))
    {
	edge->x = xorg;
	if (left && (arc->width & 1))
	    edge->x++;
	else if (!left && !(arc->width & 1))
	    edge->x--;
	edge->stepx = 0;
	edge->e = 0;
	edge->dx = -1;
	return;
    }
    sy = Dsin(angle) * arc->height;
    if (sy < 0.0)
    {
	sy = -sy;
	signdy = -1;
    }
    else
	signdy = 1;
    cx = Dcos(angle) * arc->width;
    if (cx < 0.0)
    {
	cx = -cx;
	signdx = -1;
    }
    else
	signdx = 1;
    scale = (cx > sy) ? cx : sy;
    edge->dx = floor((cx * 32768) / scale + .5);
    edge->dy = floor((sy * 32768) / scale + .5);
    if (signdx < 0)
	edge->dx = -edge->dx;
    if (signdy < 0)
	edge->dx = -edge->dx;
    y = (arc->height & ~1);
    if (!(arc->width & 1))
	y += 2;
    if (!top)
    {
	y = -y;
	if (arc->height & 1)
	    y -= 2;
    }
    x2dy = y * edge->dx;
    if (arc->height & 1)
	x2dy += edge->dx;
    edge->dx <<= 1;
    if (arc->width & 1)
	x2dy += edge->dy;
    edge->dy <<= 1;
    if ((top && (edge->dx > 0)) || (!top && edge->dx < 0))
	edge->x = (x2dy - 1) / edge->dy;
    else
	edge->x = - ((-x2dy) / edge->dy + 1);
    edge->e = x2dy - edge->x * edge->dy;
    if ((top && (edge->dx < 0)) || (!top && (edge->dx > 0)))
	edge->e = edge->dy - edge->e + 1;
    if (edge->dx < 0)
    {
	edge->dx = -edge->dx;
	edge->deltax = -1;
    }
    else
	edge->deltax = 1;
    edge->stepx = edge->dx / edge->dy;
    if (edge->deltax < 0)
	edge->stepx = -edge->stepx;
    edge->dx = edge->dx % edge->dy;
    if (left)
	edge->x++;
    edge->x += xorg;
    if (!top)
    {
	edge->stepx = -edge->stepx;
	edge->deltax = -edge->deltax;
    }
}

void
miFillArcSliceSetup(arc, slice)
    xArc *arc;
    miArcSliceRec *slice;
{
    int angle1, angle2;

    angle1 = arc->angle1;
    if (arc->angle2 < 0)
    {
	angle2 = angle1;
	angle1 += arc->angle2;
    }
    else
	angle2 = angle1 + arc->angle2;
    while (angle1 < 0)
	angle1 += FULLCIRCLE;
    while (angle1 >= FULLCIRCLE)
	angle1 -= FULLCIRCLE;
    while (angle2 < 0)
	angle2 += FULLCIRCLE;
    while (angle2 >= FULLCIRCLE)
	angle2 -= FULLCIRCLE;
    slice->min_top_y = 0;
    slice->max_top_y = arc->height >> 1;
    slice->min_bot_y = 1 - (arc->height & 1);
    slice->max_bot_y = slice->max_top_y - 1;
    slice->flip_top = FALSE;
    slice->flip_bot = FALSE;
    slice->edge1_top = (angle1 < HALFCIRCLE);
    slice->edge2_top = (angle2 <= HALFCIRCLE);
    if ((angle2 == 0) || (angle1 == HALFCIRCLE))
    {
	if (angle2 ? slice->edge2_top : slice->edge1_top)
	    slice->min_top_y = slice->min_bot_y;
	else
	    slice->min_top_y = arc->height;
	slice->min_bot_y = 0;
    }
    else if ((angle1 == 0) || (angle2 == HALFCIRCLE))
    {
	slice->min_top_y = slice->min_bot_y;
	if (angle1 ? slice->edge1_top : slice->edge2_top)
	    slice->min_bot_y = arc->height;
	else
	    slice->min_bot_y = 0;
    }
    else if (slice->edge1_top == slice->edge2_top)
    {
	if (angle2 < angle1)
	{
	    slice->flip_top = slice->edge1_top;
	    slice->flip_bot = !slice->edge1_top;
	}
	else if (slice->edge1_top)
	{
	    slice->min_top_y = 1;
	    slice->min_bot_y = arc->height;
	}
	else
	{
	    slice->min_bot_y = 0;
	    slice->min_top_y = arc->height;
	}
    }
    miGetArcEdge(arc, angle1, &slice->edge1,
		 slice->edge1_top, !slice->edge1_top);
    miGetArcEdge(arc, angle2, &slice->edge2,
		 slice->edge2_top, slice->edge2_top);
}

#define ADDSPANS() \
    pts->x = xorg - x; \
    pts->y = yorg - y; \
    *wids = slw; \
    pts++; \
    wids++; \
    n++; \
    if (miFillArcLower(slw)) \
    { \
	pts->x = xorg - x; \
	pts->y = yorg + y + dy; \
	*wids = slw; \
	pts++; \
	wids++; \
	n++; \
    }

static void
miFillEllipseI(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    register int x, y, e, ex;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int slw;
    miFillArcRec info;
    DDXPointPtr points;
    register DDXPointPtr pts;
    int *widths;
    register int *wids;
    int n;

    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * arc->height);
    if (!points)
	return;
    widths = (int *)ALLOCATE_LOCAL(sizeof(int) * arc->height);
    if (!widths)
    {
	DEALLOCATE_LOCAL(points);
	return;
    }
    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    if (pGC->miTranslate)
    {
	xorg += pDraw->x;
	yorg += pDraw->y;
    }
    n = 0;
    pts = points;
    wids = widths;
    if (arc->width == arc->height)
    {
	while (y)
	{
	    MIFILLCIRCSTEP(slw);
	    ADDSPANS();
	}
    }
    else
    {
	while (y > 0)
	{
	    MIFILLELLSTEP(slw);
	    ADDSPANS();
	}
    }
    (*pGC->ops->FillSpans)(pDraw, pGC, n, points, widths, FALSE);
    DEALLOCATE_LOCAL(widths);
    DEALLOCATE_LOCAL(points);
}

/* should use 64-bit integers */
static void
miFillEllipseD(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    register int x, y;
    int xorg, yorg, dx, dy, slw;
    double e, yk, xk, ym, xm, ex;
    DDXPointPtr points;
    register DDXPointPtr pts;
    int *widths;
    register int *wids;
    int n;

    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * arc->height);
    if (!points)
	return;
    widths = (int *)ALLOCATE_LOCAL(sizeof(int) * arc->height);
    if (!widths)
    {
	DEALLOCATE_LOCAL(points);
	return;
    }
    /* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
    /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
    ym = (double)(arc->width * arc->width) * 8;
    xm = (double)(arc->height * arc->height) * 8;
    x = 0;
    y = arc->height >> 1;
    yorg = arc->y + y;
    dy = arc->height & 1;
    dx = arc->width & 1;
    xorg = arc->x + (arc->width >> 1) + dx;
    dx = 1 - dx;
    if (arc->height & 1)
	yk = 0;
    else
	yk = ym / 2;
    if (arc->width & 1)
    {
	xk = 0;
	e = - (xm / 8);
    }
    else
    {
	y++;
	xk = xm / 2;
	e = yk - (ym * y) - xk;
    }
    ex = -xk;
    if (pGC->miTranslate)
    {
	xorg += pDraw->x;
	yorg += pDraw->y;
    }
    n = 0;
    pts = points;
    wids = widths;
    while (y > 0)
    {
	MIFILLELLSTEP(slw);
	ADDSPANS();
    }
    (*pGC->ops->FillSpans)(pDraw, pGC, n, points, widths, FALSE);
    DEALLOCATE_LOCAL(widths);
    DEALLOCATE_LOCAL(points);
}

#define ADDSPAN(l,r) \
    if (r >= l) \
    { \
	pts->x = l; \
	pts->y = ya; \
	*wids = r - l + 1; \
	pts++; \
	wids++; \
	n++; \
    }

#define ADDSLICESPANS(flip) \
    if (!flip) \
    { \
	ADDSPAN(xl, xr); \
    } \
    else \
    { \
	xc = xorg - x; \
	ADDSPAN(xc, xr); \
	xc += slw - 1; \
	ADDSPAN(xl, xc); \
    }

static void
miFillArcSliceI(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e, ex;
    miFillArcRec info;
    miArcSliceRec slice;
    int ya, xl, xr, xc;
    int iscircle;
    DDXPointPtr points;
    register DDXPointPtr pts;
    int *widths;
    register int *wids;
    int n;

    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * arc->height);
    if (!points)
	return;
    widths = (int *)ALLOCATE_LOCAL(sizeof(int) * arc->height);
    if (!widths)
    {
	DEALLOCATE_LOCAL(points);
	return;
    }
    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice);
    MIFILLARCSETUP();
    iscircle = (arc->width == arc->height);
    if (pGC->miTranslate)
    {
	xorg += pDraw->x;
	yorg += pDraw->y;
	slice.edge1.x += pDraw->x;
	slice.edge2.x += pDraw->x;
    }
    n = 0;
    pts = points;
    wids = widths;
    while (y > 0)
    {
	if (iscircle)
	{
	    MIFILLCIRCSTEP(slw);
	}
	else
	{
	    MIFILLELLSTEP(slw);
	}
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    ya = yorg - y;
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_top);
	}
	if (miFillSliceLower(slice))
	{
	    ya = yorg + y + dy;
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_bot);
	}
    }
    (*pGC->ops->FillSpans)(pDraw, pGC, n, points, widths, FALSE);
    DEALLOCATE_LOCAL(widths);
    DEALLOCATE_LOCAL(points);
}

static void
miFillArcOld(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    SppPointPtr pPts;
    SppArcRec	sppArc;
    int		angle1, angle2, a, cpt;
#define todeg(xAngle)	(((double) (xAngle)) / 64.0)

    angle1 = arc->angle1;
    if (angle1 >= FULLCIRCLE)
	    angle1 = angle1 % FULLCIRCLE;
    else if (angle1 <= -FULLCIRCLE)
	    angle1 = - (-angle1 % FULLCIRCLE);
    sppArc.x = arc->x;
    sppArc.y = arc->y;
    sppArc.width = arc->width;
    sppArc.height = arc->height;
    sppArc.angle1 = todeg (angle1);
    sppArc.angle2 = todeg (angle2);
    if(pGC->arcMode == ArcPieSlice)
    {
	/*
	 * more than half a circle isn't convex anymore,
	 * split the arc into two pieces.
	 */
	if (abs (angle2) > FULLCIRCLE / 2) {
	    pPts = (SppPointPtr) NULL;
	    a = angle2 > 0 ? FULLCIRCLE / 2 : - FULLCIRCLE / 2;
	    sppArc.angle2 = todeg (a);
	    if (cpt = miGetArcPts(&sppArc, 0, &pPts))
		miFillSppPoly(pDraw, pGC, cpt, pPts, 0, 0, 0.0, 0.0);
	    xfree (pPts);
	    angle1 += a;
	    if (angle1 >= FULLCIRCLE)
		angle1 -= FULLCIRCLE;
	    else if (angle1 <= -FULLCIRCLE)
		angle1 += FULLCIRCLE;
	    angle2 -= a;
	    sppArc.angle1 = todeg(angle1);
	    sppArc.angle2 = todeg(angle2);
	}
	if (!(pPts = (SppPointPtr)xalloc(sizeof(SppPointRec))))
	    return;
	if(cpt = miGetArcPts(&sppArc, 1, &pPts))
	{
	    pPts[0].x = sppArc.x + sppArc.width/2;
	    pPts[0].y = sppArc.y + sppArc.height /2;
	    miFillSppPoly(pDraw, pGC, cpt + 1, pPts, 0, 0, 0.0, 0.0);
	}
	xfree(pPts);
    }
    else /* Chord */
    {
	pPts = (SppPointPtr)NULL;
	if(cpt = miGetArcPts(&sppArc, 0, &pPts))
	    miFillSppPoly(pDraw, pGC, cpt, pPts, 0, 0, 0.0, 0.0);
	xfree(pPts);
    }
}

/* MIPOLYFILLARC -- The public entry for the PolyFillArc request.
 * Since we don't have to worry about overlapping segments, we can just
 * fill each arc as it comes.  As above, we convert the arc into a set of
 * line segments and then fill the resulting polygon.
 */
void
miPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register int i;
    register xArc *arc;

    for(i = narcs, arc = parcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;;
	if ((arc->angle2 >= FULLCIRCLE) || (arc->angle2 <= -FULLCIRCLE))
	{
	    if (miCanFillArc(arc))
		miFillEllipseI(pDraw, pGC, arc);
	    else
		miFillEllipseD(pDraw, pGC, arc);
	}
	else if (pGC->arcMode == ArcPieSlice)
	{
	    if (miCanFillArc(arc))
		miFillArcSliceI(pDraw, pGC, arc);
	    else
		miFillArcOld(pDraw, pGC, arc);
	}
	else
	{
	    miFillArcOld(pDraw, pGC, arc);
	}
    }
}
