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

/* $XConsortium: mifillarc.c,v 5.2 89/10/20 13:14:36 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mifpoly.h"
#include "mi.h"
#include "mifillarc.h"

# define todeg(xAngle)	(((double) (xAngle)) / 64.0)

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

    if (!arc->width || !arc->height ||
	((arc->width == 1) && (arc->height & 1)))
	return;
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

    if (!arc->width || !arc->height ||
	((arc->width == 1) && (arc->height & 1)))
	return;
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
    int	i, cpt;
    SppPointPtr pPts;
    SppArcRec	sppArc;
    int		angle1, angle2, a;
    register xArc *arc;

    for(i = narcs, arc = parcs; --i >= 0; arc++)
    {
	angle2 = arc->angle2;
	if ((angle2 >= FULLCIRCLE) || (angle2 <= -FULLCIRCLE))
	{
	    if (miCanFillArc(arc))
		miFillEllipseI(pDraw, pGC, arc);
	    else
		miFillEllipseD(pDraw, pGC, arc);
	    continue;
	}
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
}
