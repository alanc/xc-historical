/*
 * $XConsortium: cfb8line.c,v 1.4 90/06/13 18:25:08 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfbrrop.h"

#if defined(__GNUC__) && defined(mc68020)
#define STUPID volatile
#define REARRANGE
#else
#define STUPID
#endif

#define IMPORTANT_START	do { do { do { do { do { do {
#define IMPORTANT_END	} while (0); } while (0); } while (0); } while (0); } while (0); } while (0);

#define OUTCODES_IMPLICIT(result, x, y) \
    if (x < bx1) \
	result |= OUT_LEFT; \
    else if (x >= bx2) \
	result |= OUT_RIGHT; \
    if (y < by1) \
	result |= OUT_ABOVE; \
    else if (y >= by2) \
	result |= OUT_BELOW;

#define OUTCODES(result, x, y, box) \
    if (x < box->x1) \
	result |= OUT_LEFT; \
    else if (x >= box->x2) \
	result |= OUT_RIGHT; \
    if (y < box->y1) \
	result |= OUT_ABOVE; \
    else if (y >= box->y2) \
	result |= OUT_BELOW;

int
#ifdef POLYSEGMENT
RROP_NAME(cfb8SegmentSS1Rect) (pDrawable, pGC, nseg, pSegInit, boxp)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSegInit;
    BoxPtr	boxp;
#else
RROP_NAME(cfb8LineSS1Rect) (pDrawable, pGC, mode, npt, pptInit, boxp)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int	mode;		/* Origin or Previous */
    int	npt;		/* number of points */
    DDXPointPtr pptInit;
    BoxPtr	boxp;
#endif
{
    int		    bx1, bx2, by1, by2;
    int		    x1, y1;
    int		    x2, y2;
    int		    xorg, yorg;
    char	    *addr;
    int		    nwidth;
    cfbPrivGCPtr    devPriv;
    int		    capStyle;
    int		    oc1;
    STUPID int	    oc2;
#ifndef REARRANGE
    char	    *addrb;
    int	    	    adx, ady;
    int		    e, e1, e3, len;
    int		    stepx, stepy;
    RROP_DECLARE
#endif

#ifndef POLYSEGMENT
    DDXPointPtr	    ppt;
#else
    xSegment	    *pSeg;
#endif

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
#ifndef REARRANGE
    RROP_FETCH_GCPRIV(devPriv);
#endif
    capStyle = pGC->capStyle;
    bx1 = boxp->x1;
    by1 = boxp->y1;
    bx2 = boxp->x2;
    by2 = boxp->y2;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addr = (char *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind);
    }
    else
    {
	addr = (char *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nwidth = (int)(((PixmapPtr)pDrawable)->devKind);
    }
    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef POLYSEGMENT
    pSeg = pSegInit;
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    oc2 = 0;
    OUTCODES_IMPLICIT (oc2, x2, y2);
    --npt;
    while (npt--)
#endif
    {
#ifdef POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
	oc1 = 0;
	oc2 = 0;
	OUTCODES_IMPLICIT (oc1, x1, y1);
	OUTCODES_IMPLICIT (oc2, x2, y2);
#else
	x1 = x2;
	y1 = y2;
	oc1 = oc2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
	oc2 = 0;
	OUTCODES_IMPLICIT (oc2, x2, y2);
#endif
	if (oc1 & oc2)
	    continue;
#ifdef REARRANGE
	{
	register int e, e1, e3, len;
	register int stepx, stepy;
	int adx, ady;
#endif

	stepx = 1;
	if ((adx = x2 - x1) < 0)
	{
	    adx = -adx;
	    stepx = -1;
	}
	stepy = nwidth;
	if ((ady = y2 - y1) < 0)
	{
	    ady = -ady;
	    stepy = -nwidth;
	}
	/* tricky here - rename the values so that the same code
	 * can be used for X major and Y major lines
 	 */
	if (adx < ady)
	{
	    int	t;
	    t = adx;
	    adx = ady;
	    ady = t;

	    t = stepy;
	    stepy = stepx;
	    stepx = t;
	}
	/* bresenham terms - e3 = e2 - e1 saves a branch */
	e1 = ady << 1;
	e3 = - (adx << 1);
	e = - adx;
	len = adx;

	/* shorten the line to avoid pixelating the end point, except
	 * for segments in non CapNotLast cap style.  Final line endpoint
	 * will be done outside the loop, saving an additional test
	 * inside.
	 */
#ifdef POLYSEGMENT
	if (capStyle == CapNotLast)
#endif
	    if (!len--)
		continue;

#ifdef REARRANGE
	{
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
#endif

	addrb = addr + (y1 * nwidth) + x1;

	/* clip the line if necessary.  This code is heavily
	 * optimized for the case where lines are either
	 * fully clipped or unclipped; rearranging things
	 * would allow clipped lines to run a bit faster perhaps
	 */
	if (oc1 | oc2)
	{
	    STUPID int	xt = x1, yt = y1, lent;
#ifdef POLYSEGMENT
#define shorten	(capStyle == CapNotLast)
#else
#define shorten	TRUE
#endif
	    e = cfbClipLine (&xt, &yt, &lent, x2, y2, boxp, shorten);
#undef shorten
	    len = lent;
	    addrb = addr + (yt * nwidth) + xt;

	    /* make sure the line is still visible */
	    if (len < 0)
		continue;
	}

#ifndef REARRANGE
	if (!ady)
	{
#define body	{ RROP_SOLID(addrb); addrb += stepx; }
	    while (len >= 4)
	    {
		body body body body
		len -= 4;
	    }
	    switch (len)
	    {
	    case  3: body case 2: body case 1: body
	    }
#undef body
	}
	else
#endif
	{
#define body {\
	    	RROP_SOLID(addrb); \
	    	addrb += stepx; \
	    	e += e1; \
	    	if (e >= 0) \
	    	{ \
		    addrb += stepy; \
		    e += e3; \
	     	 } \
	    }

#ifdef LARGE_INSTRUCTION_CACHE
	    while (len >= 4)
	    {
	    	body body body body
	    	len -= 4;
	    }
	    switch (len)
	    {
	    case  3: body case 2: body case 1: body
	    }
#else
	    IMPORTANT_START

	    while ((len -= 2) >= 0)
	    {
	    	body body
	    }
	    if (len & 1)
	    	body;

	    IMPORTANT_END

#endif
	}
	RROP_SOLID(addrb);
#undef body

#ifdef REARRANGE
	}
	}
#endif

    }
    /* special case for the last point; draw it only when the first
     * and last don't join
     */
#ifndef POLYSEGMENT
    if (capStyle != CapNotLast && !oc2 &&
	(x2 != pptInit->x || y2 != pptInit->y))
    {
#ifdef REARRANGE
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
#endif
	addrb = addr + (y2 * nwidth) + x2;
	RROP_SOLID (addrb);
    }
#endif
    return -1;
}

#if RROP == GXset && !defined (POLYSEGMENT)

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )
#define ceiling(m,n)  (((m)-1)/(n) + 1)
#define SignTimes(sign,n)   (((sign) < 0) ? -(n) : (n))

static
cfbClipPoint (oc, xp, yp, dx, dy, boxp)
    int	oc;
    int	*xp, *yp;
    BoxPtr  boxp;
{
    int	x, y;
    int	adx, ady, signdx, signdy;
    int	utmp;
    
    signdx = 1;
    if (dx < 0)
    {
    	signdx = -1;
    	dx = -dx;
    }
    signdy = 1;
    if (dy  < 0)
    {
    	signdy = -1;
    	dy = -dy;
    }
    if (oc & (OUT_LEFT | OUT_RIGHT))
    {
    	if (oc & OUT_LEFT)
    	{
	    x = boxp->x1;
	    utmp = x - *xp;
    	}
    	else
    	{
	    x = boxp->x2 - 1;
	    utmp = *xp - x;
    	}
    	utmp *= dy;
	if (dy > dx)
	{
	    utmp = (utmp << 1) - dy + 1;
	    y = *yp + SignTimes(signdy, ceiling(utmp, (dx << 1)));
	}
	else
	{
    	    y = *yp + SignTimes(signdy, round(utmp, dx));
	}
	oc = 0;
	OUTCODES (oc, x, y, boxp);
    }
    if (oc & (OUT_ABOVE | OUT_BELOW))
    {
    	if (oc & OUT_ABOVE)
    	{
    	    y = boxp->y1;
    	    utmp = y - *yp;
    	}
    	else
    	{
    	    y = boxp->y2 - 1;
    	    utmp = *yp - y;
    	}
	utmp *= dx;
	if (dx > dy)
	{
	    utmp = (utmp << 1) - dx + 1;
	    x = *xp + SignTimes(signdx, ceiling(utmp, (dy << 1)));
	}
	else
	{
	    x = *xp + SignTimes(signdx, round(utmp, dy));
	}
	oc = 0;
	OUTCODES (oc, x, y, boxp);
    }
    *xp = x;
    *yp = y;
    return oc;
}

cfbClipLine (x1p, y1p, lenp, x2, y2, boxp, shorten)
    int		*x1p, *y1p;
    int		*lenp;
    int		x2, y2;
    BoxPtr	boxp;
    Bool	shorten;
{
    int		    x1, y1;
    int		    oc1, oc2;
    int		    signdx, signdy, axis, e, e1, e3, len;
    int		    adx, ady;
    
    x1 = *x1p;
    y1 = *y1p;
    signdx = 1;
    if ((adx = x2 - x1) < 0)
    {
	adx = -adx;
	signdx = -1;
    }
    signdy = 1;
    if ((ady = y2 - y1) < 0)
    {
	ady = -ady;
	signdy = -1;
    }
    axis = X_AXIS;
    if (adx <= ady)
    {
	int	t;

	t = adx;
	adx = ady;
	ady = t;

	axis = Y_AXIS;
    }
    e1 = ady << 1;
    e3 = - (adx << 1);
    e = - adx;
    len = adx;
    oc1 = 0;
    oc2 = 0;
    OUTCODES (oc1, x1, y1, boxp);
    OUTCODES (oc2, x2, y2, boxp);
    if (oc2)
    {
	int xt = x2, yt = y2;
	int	dx = x2 - x1, dy = y2 - y1;
	int change;

	oc2 = cfbClipPoint (oc2, &xt, &yt, -dx, -dy, boxp);
	if (axis == Y_AXIS)
	    change = y2 - yt;
	else
	    change = x2 - xt;
	if (change < 0)
	    change = -change;
	len -= change;
    } else if (shorten)
	--len;
    if (oc1)
    {
	int	xt = x1, yt = y1;
	int	dx = x2 - x1, dy = y2 - y1;
	int	changex, changey;

	oc1 = cfbClipPoint (oc1, &xt, &yt, dx, dy, boxp);
	changex = x1 - xt;
	if (changex < 0)
	    changex = -changex;
	changey = y1 - yt;
	if (changey < 0)
	    changey = -changey;
	if (axis == X_AXIS)
	{
	    len -= changex;
	    e = e + changey * e3 + changex * e1;
	}
	else
	{
	    len -= changey;
	    e = e + changex * e3 + changey * e1;
	}
	*x1p = xt;
	*y1p = yt;
    }
    if (oc1 | oc2)
	len = -1;
    *lenp = len;
    return e;
}

extern int cfb8LineSS1RectCopy(), cfb8LineSS1RectXor(), cfb8LineSS1RectGeneral(); 

void
cfb8LineSS (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pptInit;
{
    int		    (*func)();
    cfbPrivGCPtr    devPriv;
    BoxPtr	    boxp;
    int		    nbox;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8LineSS1RectCopy;
	break;
    case GXxor:
	func = cfb8LineSS1RectXor;
	break;
    default:
	func = cfb8LineSS1RectGeneral;
	break;
    }
    for (boxp = REGION_RECTS (devPriv->pCompositeClip),
	 nbox = REGION_NUM_RECTS (devPriv->pCompositeClip);
 	 nbox;
 	 nbox--, boxp++)
    {
	(*func) (pDrawable, pGC, mode, npt, pptInit, boxp);
    }
}

extern int cfb8SegmentSS1RectCopy(), cfb8SegmentSS1RectXor(), cfb8SegmentSS1RectGeneral(); 

void
cfb8SegmentSS (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nseg;
    xSegment	    *pSegInit;
{
    int		    (*func)();
    cfbPrivGCPtr    devPriv;
    BoxPtr	    boxp;
    int		    nbox;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8SegmentSS1RectCopy;
	break;
    case GXxor:
	func = cfb8SegmentSS1RectXor;
	break;
    default:
	func = cfb8SegmentSS1RectGeneral;
	break;
    }
    for (boxp = REGION_RECTS (devPriv->pCompositeClip),
	 nbox = REGION_NUM_RECTS (devPriv->pCompositeClip);
 	 nbox;
 	 nbox--, boxp++)
    {
	(*func) (pDrawable, pGC, nseg, pSegInit, boxp);
    }
}

#endif
