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

/* $XConsortium: mizerarc.c,v 5.15 89/09/15 13:12:50 rws Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include <math.h>
#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mi.h"
#include "mizerarc.h"

Bool
miZeroArcSetup(arc, info, ok360)
    register xArc *arc;
    register miZeroArcRec *info;
    Bool ok360;
{
    int l;
    int angle1, angle2;
    int startseg, endseg;
    int startAngle, endAngle;
    int i, overlap;

    l = arc->width & 1;
    if (arc->width == arc->height)
    {
	info->alpha = 4;
	info->beta = 4;
	info->k1 = -8;
	info->k3 = -16;
	info->b = 12;
	info->a = (arc->width << 2) - 12;
	info->d = 17 - (arc->width << 1);
	if (l)
	{
	    info->b -= 4;
	    info->a += 4;
	    info->d -= 7;
	}
    }
    else
    {
	info->alpha = (arc->width * arc->width) << 2;
	info->beta = (arc->height * arc->height) << 2;
	info->k1 = -(info->beta << 1);
	info->k3 = info->k1 - (info->alpha << 1);
	info->b = info->beta * (3 - l);
	info->a = (info->alpha * arc->height) - info->b;
	info->d = info->b - (info->a >> 1) - ((info->beta >> 2) * (2 + l)) +
		 (info->alpha >> 2);
    }
    info->x = arc->width ? 1 : 0;
    info->y = 0;
    info->dx1 = 1;
    info->dy1 = 0;
    info->w = (arc->width + 1) >> 1;
    info->h = arc->height >> 1;
    info->xorg = arc->x + (arc->width >> 1);
    info->yorg = arc->y;
    info->xorgo = info->xorg + l;
    info->yorgo = info->yorg + arc->height;
    angle1 = arc->angle1;
    angle2 = arc->angle2;
    if (angle2 > FULLCIRCLE)
	angle2 = FULLCIRCLE;
    else if (angle2 < -FULLCIRCLE)
	angle2 = -FULLCIRCLE;
    if (angle2 < 0)
    {
	startAngle = angle1 + angle2;
	endAngle = angle1;
    }
    else
    {
	startAngle = angle1;
	endAngle = angle1 + angle2;
    }
    if (startAngle < 0)
	startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
    if (startAngle >= FULLCIRCLE)
	startAngle = startAngle % FULLCIRCLE;
    if (endAngle < 0)
	endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
    if (endAngle >= FULLCIRCLE)
	endAngle = endAngle % FULLCIRCLE;
    info->startAngle = startAngle;
    info->endAngle = endAngle;
    if (startAngle == endAngle)
    {
	if (!angle2)
	{
	    info->initialMask = 0;
	    info->h = -1;
	    info->w = -1;
	    return FALSE;
	}
	if (ok360)
	{
	    info->initialMask = 0xf;
	    return TRUE;
	}
    }
    startseg = startAngle / OCTANT;
    if (!arc->height || (((startseg + 1) & 2) && arc->width))
    {
	info->startx = Dcos((double)startAngle/64.0) * (arc->width / 2.0);
	if (info->startx < 0)
	    info->startx = -info->startx;
	info->starty = -1;
    }
    else
    {
	info->starty = Dsin((double)startAngle/64.0) * (arc->height / 2.0);
	if (info->starty < 0)
	    info->starty = -info->starty;
	info->starty = info->h - info->starty;
	info->startx = 65536;
    }
    endseg = endAngle / OCTANT;
    if (!arc->height || (((endseg + 1) & 2) && arc->width))
    {
	info->endx = Dcos((double)endAngle/64.0) * (arc->width / 2.0);
	if (info->endx < 0)
	    info->endx = -info->endx;
	info->endy = -1;
    }
    else
    {
	info->endy = Dsin((double)endAngle/64.0) * (arc->height / 2.0);
	if (info->endy < 0)
	    info->endy = -info->endy;
	info->endy = info->h - info->endy;
	info->endx = 65536;
    }
    info->initialMask = 0;
    for (i = 0; i < 4; i++)
    {
	if ((endAngle <= startAngle) ?
	    ((i * QUADRANT <= endAngle) || ((i + 1) * QUADRANT > startAngle)) :
	    ((i * QUADRANT <= endAngle) && ((i + 1) * QUADRANT > startAngle)))
	    info->initialMask |= (1 << i);
    }
    info->startMask = info->initialMask;
    info->endMask = info->initialMask;
    startseg >>= 1;
    endseg >>= 1;
    overlap = (endseg == startseg) && (endAngle <= startAngle);
    if (startseg & 1)
    {
	if  (!overlap)
	    info->initialMask &= ~(1 << startseg);
	if (info->startx > info->endx || info->starty > info->endy)
	    info->endMask &= ~(1 << startseg);
    }
    else
    {
	info->startMask &= ~(1 << startseg);
	if ((info->startx < info->endx || info->starty < info->endy ||
	    (info->startx == info->endx && info->starty == info->endy)) &&
	    !overlap)
	    info->endMask &= ~(1 << startseg);
    }
    if (endseg & 1)
    {
	info->endMask &= ~(1 << endseg);
	if ((info->startx > info->endx || info->starty > info->endy) &&
	    !overlap)
	    info->startMask &= ~(1 << endseg);
    }
    else
    {
	if (!overlap)
	    info->initialMask &= ~(1 << endseg);
	if (info->startx < info->endx || info->starty < info->endy)
	    info->startMask &= ~(1 << endseg);
    }
    if (!info->startx)
	info->initialMask = info->startMask;
    return FALSE;
}

DDXPointPtr
miZeroArcPts(arc, pts)
    xArc *arc;
    register DDXPointPtr pts;
{
    miZeroArcRec info;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx1, dy1;

    (void)miZeroArcSetup(arc, &info, FALSE);
    x = info.x;
    y = info.y;
    k1 = info.k1;
    k3 = info.k3;
    a = info.a;
    b = info.b;
    d = info.d;
    dx1 = info.dx1;
    dy1 = info.dy1;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	if (mask & 1)
	{
	    pts->x = info.xorg;
	    pts->y = info.yorg;
	    pts++;
	}
	if (mask & 4)
	{
	    pts->x = info.xorg;
	    pts->y = info.yorgo;
	    pts++;
	}
    }
    if (!info.endx)
	mask = info.endMask;
    while (y < info.h)
    {
	if (a < 0)
	{
	    dx1 = 0;
	    dy1 = 1;
	    k1 = info.alpha << 1;
	    k3 = -k3;
	    b = b + a - info.alpha;
	    d = b - (a >> 1) - d + (k3 >> 3);
	    a = (info.alpha - info.beta) - a;
	}
	if ((x == info.startx) || (y == info.starty))
	    mask = info.startMask;
	if (mask & 1)
	{
	    pts->x = info.xorg + x;
	    pts->y = info.yorg + y;
	    pts++;
	}
	if (mask & 2)
	{
	    pts->x = info.xorgo - x;
	    pts->y = info.yorg + y;
	    pts++;
	}
	if (mask & 4)
	{
	    pts->x = info.xorgo - x;
	    pts->y = info.yorgo - y;
	    pts++;
	}
	if (mask & 8)
	{
	    pts->x = info.xorg + x;
	    pts->y = info.yorgo - y;
	    pts++;
	}
	if ((x == info.endx) || (y == info.endy))
	    mask = info.endMask;
	b -= k1;
	if (d < 0)
	{
	    x += dx1;
	    y += dy1;
	    a += k1;
	    d += b;
	}
	else
	{
	    x++;
	    y++;
	    a += k3;
	    d -= a;
	}
    }
    for (; x <= info.w; x++)
    {
	if (mask & 1)
	{
	    pts->x = info.xorg + x;
	    pts->y = info.yorg + y;
	    pts++;
	}
	if (mask & 2)
	{
	    pts->x = info.xorgo - x;
	    pts->y = info.yorg + y;
	    pts++;
	}
	if (!arc->height || (arc->height & 1))
	{
	    if (mask & 4)
	    {
		pts->x = info.xorgo - x;
		pts->y = info.yorgo - y;
		pts++;
	    }
	    if (mask & 8)
	    {
		pts->x = info.xorg + x;
		pts->y = info.yorgo - y;
		pts++;
	    }
	}
    }
    return pts;
}

#define DoPix(idx, xval, yval) \
    if (mask & (1 << idx)) \
    { \
	arcPts[idx]->x = xval; \
	arcPts[idx]->y = yval; \
	arcPts[idx]++; \
    }

static void
miZeroArcDashPts(pGC, arc, points, maxPts, evenPts, oddPts)
    GCPtr pGC;
    register xArc *arc;
    int maxPts;
    register DDXPointPtr points, *evenPts, *oddPts;
{
    miZeroArcRec info;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx1, dy1;
    unsigned char *pDash;
    int numInDashList, dashIndex, dashOffset, dashRemaining;
    DDXPointPtr arcPts[4];
    DDXPointPtr startPt;
    DDXPointPtr pt, lastPt, *pts;
    int i, delta, ptsdelta, seg, startseg;

    if (!arc->angle2)
	return;
    for (i = 0; i < 4; i++)
	arcPts[i] = points + (i * maxPts);
    (void)miZeroArcSetup(arc, &info, FALSE);
    x = info.x;
    y = info.y;
    k1 = info.k1;
    k3 = info.k3;
    a = info.a;
    b = info.b;
    d = info.d;
    dx1 = info.dx1;
    dy1 = info.dy1;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(0, info.xorg, info.yorg);
	DoPix(2, info.xorg, info.yorgo);
    }
    startseg = info.startAngle / QUADRANT;
    startPt = arcPts[startseg];
    if (!info.endx)
	mask = info.endMask;
    while (y < info.h)
    {
	if (a < 0)
	{
	    dx1 = 0;
	    dy1 = 1;
	    k1 = info.alpha << 1;
	    k3 = -k3;
	    b = b + a - info.alpha;
	    d = b - (a >> 1) - d + (k3 >> 3);
	    a = (info.alpha - info.beta) - a;
	}
	if ((x == info.startx) || (y == info.starty))
	{
	    mask = info.startMask;
	    startPt = arcPts[startseg];
	}
	DoPix(0, info.xorg + x, info.yorg + y);
	DoPix(1, info.xorgo - x, info.yorg + y);
	DoPix(2, info.xorgo - x, info.yorgo - y);
	DoPix(3, info.xorg + x, info.yorgo - y);
	if ((x == info.endx) || (y == info.endy))
	    mask = info.endMask;
	b -= k1;
	if (d < 0)
	{
	    x += dx1;
	    y += dy1;
	    a += k1;
	    d += b;
	}
	else
	{
	    x++;
	    y++;
	    a += k3;
	    d -= a;
	}
    }
    if ((x == info.startx) || (y == info.starty))
	startPt = arcPts[startseg];
    for (; x <= info.w; x++)
    {
	if (x == info.startx)
	    startPt = arcPts[startseg];
	DoPix(0, info.xorg + x, info.yorg + y);
	DoPix(1, info.xorgo - x, info.yorg + y);
	if (!arc->height || (arc->height & 1))
	{
	    DoPix(2, info.xorgo - x, info.yorgo - y);
	    DoPix(3, info.xorg + x, info.yorgo - y);
	}
    }
    pDash = (unsigned char *) pGC->dash;
    numInDashList = pGC->numInDashList;
    dashIndex = 0;
    dashOffset = 0;
    miStepDash((int)pGC->dashOffset, &dashIndex, pDash,
	       numInDashList, &dashOffset);
    dashRemaining = pDash[dashIndex] - dashOffset;
    for (i = 0; i < 5; i++)
    {
	seg = (startseg + i) & 3;
	pt = points + (seg * maxPts);
	if (seg & 1)
	{
	    lastPt = arcPts[seg];
	    delta = 1;
	    if (i == 0)
		pt = startPt;
	    else if (i == 4)
	    {
		if (startPt == pt)
		    break;
		lastPt = startPt - 1;
	    }
	}
	else
	{
	    lastPt = pt - 1;
	    pt = arcPts[seg] - 1;
	    delta = -1;
	    if (i == 0)
	    {
		if (startPt < pt)
		    pt = startPt;
	    }
	    else if (i == 4)
	    {
		if (startPt > pt)
		    break;
		lastPt = startPt;
	    }
	}
	while (pt != lastPt)
	{
	    if (dashIndex & 1)
	    {
		pts = oddPts;
		ptsdelta = -1;
	    }
	    else
	    {
		pts = evenPts;
		ptsdelta = 1;
	    }
	    while ((pt != lastPt) && --dashRemaining >= 0)
	    {
		**pts = *pt;
		*pts += ptsdelta;
		pt += delta;
	    }
	    if (dashRemaining <= 0)
	    {
		if (++dashIndex == numInDashList)
		    dashIndex = 0;
		dashRemaining = pDash[dashIndex];
	    }
	}
    }
}

void
miZeroPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    int maxPts = 0;
    int n, maxw;
    register xArc *arc;
    register int i;
    DDXPointPtr points, pts, oddPts;
    register DDXPointPtr pt;
    int numPts;
    Bool dospans;
    int *widths;
    unsigned long fgPixel = pGC->fgPixel;

    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (!miCanZeroArc(arc))
	    miPolyArc(pDraw, pGC, 1, arc);
	else
	{
	    if (arc->width > arc->height)
		n = arc->width + (arc->height >> 1);
	    else
		n = arc->height + (arc->width >> 1);
	    if (n > maxPts)
		maxPts = n;
	}
    }
    if (!maxPts)
	return;
    numPts = maxPts << 2;
    dospans = (pGC->lineStyle != LineSolid) || (pGC->fillStyle != FillSolid);
    if (dospans)
    {
	widths = (int *)ALLOCATE_LOCAL(sizeof(int) * numPts);
	if (!widths)
	    return;
	maxw = 0;
    }
    if (pGC->lineStyle != LineSolid)
	numPts <<= 1;
    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * numPts);
    if (!points)
    {
	if (dospans)
	{
	    DEALLOCATE_LOCAL(widths);
	}
	return;
    }
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    if (pGC->lineStyle == LineSolid)
		pts = miZeroArcPts(arc, points);
	    else
	    {
		pts = points;
		oddPts = &points[(numPts >> 1) - 1];
		miZeroArcDashPts(pGC, arc, oddPts + 1, maxPts, &pts, &oddPts);
	    }
	    n = pts - points;
	    if (!dospans)
		(*pGC->ops->PolyPoint)(pDraw, pGC, CoordModeOrigin, n, points);
	    else
	    {
		if (n > maxw)
		{
		    while (maxw < n)
			widths[maxw++] = 1;
		}
		if (pGC->miTranslate)
		{
		    for (pt = points; pt != pts; pt++)
		    {
			pt->x += pDraw->x;
			pt->y += pDraw->y;
		    }
		}
		(*pGC->ops->FillSpans)(pDraw, pGC, n, points, widths, FALSE);
	    }
	    if (pGC->lineStyle != LineDoubleDash)
		continue;
	    if ((pGC->fillStyle == FillSolid) ||
		(pGC->fillStyle == FillStippled))
	    {
		DoChangeGC(pGC, GCForeground, (XID *)&pGC->bgPixel, 0);
		ValidateGC(pDraw, pGC);
	    }
	    pts = &points[numPts >> 1];
	    oddPts++;
	    n = pts - oddPts;
	    if (!dospans)
		(*pGC->ops->PolyPoint)(pDraw, pGC, CoordModeOrigin, n, oddPts);
	    else
	    {
		if (n > maxw)
		{
		    while (maxw < n)
			widths[maxw++] = 1;
		}
		if (pGC->miTranslate)
		{
		    for (pt = oddPts; pt != pts; pt++)
		    {
			pt->x += pDraw->x;
			pt->y += pDraw->y;
		    }
		}
		(*pGC->ops->FillSpans)(pDraw, pGC, n, oddPts, widths, FALSE);
	    }
	    if ((pGC->fillStyle == FillSolid) ||
		(pGC->fillStyle == FillStippled))
	    {
		DoChangeGC(pGC, GCForeground, (XID *)&fgPixel, 0);
		ValidateGC(pDraw, pGC);
	    }
	}
    }
    if (dospans)
    {
	DEALLOCATE_LOCAL(widths);
    }
    DEALLOCATE_LOCAL(points);
}
