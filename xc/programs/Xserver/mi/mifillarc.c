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

/* $XConsortium: mifillarc.c,v 5.0 89/09/20 18:55:54 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mi.h"
#include "mifillarc.h"

void
miFillArcSetup(arc, info)
    register xArc *arc;
    register miFillArcRec *info;
{
    if (arc->width == arc->height)
    {
	/* (2x - 2xorg)^2 = d^2 - (2y - 2yorg)^2 */
	/* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
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

    if (!arc->width || !arc->height)
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

    if (!arc->width || !arc->height)
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
