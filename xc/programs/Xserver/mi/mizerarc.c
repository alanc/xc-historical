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

/* $XConsortium: mizerarc.c,v 5.6 89/09/05 00:45:04 keith Exp $ */

#include <math.h>
#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mi.h"
#include "mizerarc.h"

void
miZeroCircleSetup(arc, info)
    register xArc *arc;
    register miZeroCircleRec *info;
{
    int angle1, angle2;
    int startseg, endseg;
    int startAngle, endAngle;
    int i, r, overlap;

    r = arc->width >> 1;
    info->y = r;
    info->xorg = arc->x + info->y;
    info->xorgo = info->xorg;
    info->yorg = arc->y + info->y;
    info->yorgo = info->yorg;
    if (arc->width & 1)
    {
	info->xorgo++;
	info->yorgo++;
	info->x = 1;
	info->y++;
	info->d = 7 - (int)(arc->width << 1);
	info->dn = 4;
	info->dp = 6;
    }
    else
    {
	info->x = 0;
	info->d = 3 - (int)arc->width;
	info->dn = 6;
	info->dp = 10;
    }
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
	info->startx = -1;
	info->endx = -1;
	if (angle2)
	    info->initialMask = 0xff;
	else
	    info->initialMask = 0;
	return;
    }
    startseg = startAngle / EIGHTH;
    if ((startseg + 1) & 2)
	info->startx = Dcos((double)startAngle/64.0) * r;
    else
	info->startx = Dsin((double)startAngle/64.0) * r;
    if (info->startx < 0)
	info->startx = -info->startx;
    endseg = endAngle / EIGHTH;
    if ((endseg + 1) & 2)
	info->endx = Dcos((double)endAngle/64.0) * r;
    else
	info->endx = Dsin((double)endAngle/64.0) * r;
    if (info->endx < 0)
	info->endx = -info->endx;
    info->initialMask = 0;
    for (i = 0; i < 8; i++)
    {
	if ((endAngle < startAngle) ?
	    ((i * EIGHTH <= endAngle) || ((i + 1) * EIGHTH > startAngle)) :
	    ((i * EIGHTH <= endAngle) && ((i + 1) * EIGHTH > startAngle)))
	    info->initialMask |= (1 << i);
    }
    info->startMask = info->initialMask;
    info->endMask = info->initialMask;
    overlap = (endseg == startseg) && (endAngle < startAngle);
    if (startseg & 1)
    {
	info->startMask &= ~(1 << startseg);
	if ((info->startx < info->endx) && !overlap)
	    info->endMask &= ~(1 << startseg);
    }
    else
    {
	if (info->startx > info->endx)
	    info->endMask &= ~(1 << startseg);
	if  (!overlap)
	    info->initialMask &= ~(1 << startseg);
    }
    if (endseg & 1)
    {
	if (info->startx < info->endx)
	    info->startMask &= ~(1 << endseg);
	if (!overlap)
	    info->initialMask &= ~(1 << endseg);
    }
    else
    {
	info->endMask &= ~(1 << endseg);
	if ((info->startx > info->endx) && !overlap)
	    info->startMask &= ~(1 << endseg);
    }
    if ((arc->width & 1) && !info->startx)
    {
	if (!info->endx)
	    info->startMask = info->endMask;
	info->startx = 1;
    }
}

static DDXPointPtr
miZeroCirclePts(arc, pts)
    register xArc *arc;
    register DDXPointPtr pts;
{
    register int x, y, d, dn, dp;
    register int mask;
    miZeroCircleRec info;

    miZeroCircleSetup(arc, &info);
    y = info.y;
    d = info.d;
    dn = info.dn;
    dp = info.dp;
    mask = info.initialMask;
    for (x = info.x; x <= y; x++)
    {
	if (x == info.startx)
	    mask = info.startMask;
	if (mask & 1)
	{
	    pts->x = info.xorg + y;
	    pts->y = info.yorgo - x;
	    pts++;
	}
	if (mask & 4)
	{
	    pts->x = info.xorgo - x;
	    pts->y = info.yorgo - y;
	    pts++;
	}
	if (mask & 16)
	{
	    pts->x = info.xorgo - y;
	    pts->y = info.yorg + x;
	    pts++;
	}
	if (mask & 64)
	{
	    pts->x = info.xorg + x;
	    pts->y = info.yorg + y;
	    pts++;
	}
	if (x == y)
	    break;
	if (x)
	{
	    if (mask & 2)
	    {
		pts->x = info.xorg + x;
		pts->y = info.yorgo - y;
		pts++;
	    }
	    if (mask & 8)
	    {
		pts->x = info.xorgo - y;
		pts->y = info.yorgo - x;
		pts++;
	    }
	    if (mask & 32)
	    {
		pts->x = info.xorgo - x;
		pts->y = info.yorg + y;
		pts++;
	    }
	    if (mask & 128)
	    {
		pts->x = info.xorg + y;
		pts->y = info.yorg + x;
		pts++;
	    }
	}
	if (x == info.endx)
	    mask = info.endMask;
	if (d < 0)
	{
	    d += (x << 2) + dn;
	}
	else
	{
	    d += ((x - y) << 2) + dp;
	    y--;
	}
    }
    return pts;
}

#define DoPix(idx, xval, yval) \
    if (mask & (1 << idx)) \
    { \
	circPts[idx]->x = xval; \
	circPts[idx]->y = yval; \
	circPts[idx]++; \
    }

static void
miZeroCircleDashPts(pGC, arc, points, evenPts, oddPts)
    GCPtr pGC;
    register xArc *arc;
    register DDXPointPtr points, *evenPts, *oddPts;
{
    register int x, y, d, dn, dp;
    register int mask;
    miZeroCircleRec info;
    unsigned char *pDash;
    int numInDashList, dashIndex, dashOffset, dashRemaining;
    DDXPointPtr circPts[8];
    DDXPointPtr pt, lastPt, *pts;
    int i, delta, ptsdelta, seg;

    miZeroCircleSetup(arc, &info);
    for (i = 0; i < 8; i++)
	circPts[i] = points + (i * arc->width);
    y = info.y;
    d = info.d;
    dn = info.dn;
    dp = info.dp;
    mask = info.initialMask;
    for (x = info.x; x <= y; x++)
    {
	if (x == info.startx)
	    mask = info.startMask;
	DoPix(0, info.xorg + y, info.yorgo - x);
	DoPix(2, info.xorgo - x, info.yorgo - y);
	DoPix(4, info.xorgo - y, info.yorg + x);
	DoPix(6, info.xorg + x, info.yorg + y);
	if (x == y)
	    break;
	if (x)
	{
	    DoPix(1, info.xorg + x, info.yorgo - y);
	    DoPix(3, info.xorgo - y, info.yorgo - x);
	    DoPix(5, info.xorgo - x, info.yorg + y);
	    DoPix(7, info.xorg + y, info.yorg + x);
	}
	if (x == info.endx)
	    mask = info.endMask;
	if (d < 0)
	{
	    d += (x << 2) + dn;
	}
	else
	{
	    d += ((x - y) << 2) + dp;
	    y--;
	}
    }
    pDash = (unsigned char *) pGC->dash;
    numInDashList = pGC->numInDashList;
    dashIndex = 0;
    dashOffset = 0;
    miStepDash(pGC->dashOffset, &dashIndex, pDash,
	       numInDashList, &dashOffset);
    dashRemaining = pDash[dashIndex] - dashOffset;
    for (i = 0; i < 8; i++)
    {
	seg = ((info.startAngle / EIGHTH) + i) & 7;
	if (seg & 1)
	{
	    lastPt = points + (seg * arc->width) - 1;
	    pt = circPts[seg] - 1;
	    delta = -1;
	}
	else
	{
	    pt = points + (seg * arc->width);
	    lastPt = circPts[seg];
	    delta = 1;
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
    int maxWidth = 0;
    int n;
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
	if (arc->width != arc->height)
	    miPolyArc(pDraw, pGC, 1, arc);
	else if (arc->width > maxWidth)
	    maxWidth = arc->width;
    }
    if (!maxWidth)
	return;
    numPts = maxWidth << 3;
    dospans = (pGC->lineStyle != LineSolid) || (pGC->fillStyle != FillSolid);
    if (dospans)
    {
	widths = (int *)ALLOCATE_LOCAL(sizeof(int) * numPts);
	if (!widths)
	    return;
	maxWidth = 0;
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
	if (arc->width == arc->height)
	{
	    if (pGC->lineStyle == LineSolid)
		pts = miZeroCirclePts(arc, points);
	    else
	    {
		pts = points;
		oddPts = &points[(numPts >> 1) - 1];
		miZeroCircleDashPts(pGC, arc, oddPts + 1, &pts, &oddPts);
	    }
	    n = pts - points;
	    if (!dospans)
		(*pGC->ops->PolyPoint)(pDraw, pGC, CoordModeOrigin, n, points);
	    else
	    {
		if (n > maxWidth)
		{
		    while (maxWidth < n)
			widths[maxWidth++] = 1;
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
		if (n > maxWidth)
		{
		    while (maxWidth < n)
			widths[maxWidth++] = 1;
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

