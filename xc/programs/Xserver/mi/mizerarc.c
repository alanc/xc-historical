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

/* $XConsortium: mizerarc.c,v 5.1 89/09/02 17:09:25 rws Exp $ */

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
    if (endAngle > FULLCIRCLE)
	endAngle = (endAngle-1) % FULLCIRCLE + 1;
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
	    ((i * EIGHTH < endAngle) || ((i + 1) * EIGHTH >= startAngle)) :
	    ((i * EIGHTH < endAngle) && ((i + 1) * EIGHTH >= startAngle)))
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
	info->startx = 1;
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
	else if (x == info.endx)
	    mask = info.endMask;
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
	if ((x == y) || (x == 0))
	    continue;
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

void
miZeroPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    int max = 0;
    int n;
    register xArc *arc;
    register int i;
    DDXPointPtr points, pts;
    register DDXPointPtr pt;
    Bool dospans;
    int *widths;

    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (arc->width != arc->height)
	    miPolyArc(pDraw, pGC, 1, arc);
	else if (arc->width > max)
	    max = arc->width;
    }
    if (!max)
	return;
    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * max * 8);
    if (!points)
	return;
    dospans = (pGC->lineStyle != LineSolid) || (pGC->fillStyle != FillSolid);
    if (dospans)
    {
	widths = (int *)ALLOCATE_LOCAL(sizeof(int) * max * 8);
	if (!widths)
	{
	    DEALLOCATE_LOCAL(points);
	    return;
	}
	max = 0;
    }
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (arc->width == arc->height)
	{
	    pts = miZeroCirclePts(arc, points);
	    n = pts - points;
	    if (!dospans)
		(*pGC->ops->PolyPoint)(pDraw, pGC, CoordModeOrigin, n, points);
	    else
	    {
		if (n > max)
		{
		    while (max < n)
			widths[max++] = 1;
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
	}
    }
    if (dospans)
    {
	DEALLOCATE_LOCAL(widths);
    }
    DEALLOCATE_LOCAL(points);
}
