/*
 * $XConsortium: cfb8line.c,v 1.8 90/08/31 15:37:16 keith Exp $
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

#define cfbGetByteWidth(pDrawable) (((pDrawable)->type == DRAWABLE_WINDOW) ? \
    (int) (((PixmapPtr)((pDrawable)->pScreen->devPrivate))->devKind) : \
    (int)(((PixmapPtr)pDrawable)->devKind))
    
#define cfbGetByteWidthAndPointer(pDrawable, width, pointer) { \
    if ((pDrawable)->type == DRAWABLE_WINDOW) { \
	(pointer) = (char *) \
		(((PixmapPtr)((pDrawable)->pScreen->devPrivate))->devPrivate.ptr); \
	(width) = (int) \
		(((PixmapPtr)((pDrawable)->pScreen->devPrivate))->devKind); \
    } else { \
	(pointer) = (char *)(((PixmapPtr)pDrawable)->devPrivate.ptr); \
	(width) = (int)(((PixmapPtr)pDrawable)->devKind); \
    } \
}

#define cfbGetLongWidthAndPointer(pDrawable, width, pointer) { \
    if ((pDrawable)->type == DRAWABLE_WINDOW) { \
	(pointer) = (unsigned long *) \
		(((PixmapPtr)((pDrawable)->pScreen->devPrivate))->devPrivate.ptr); \
	(width) = (int) \
		(((PixmapPtr)((pDrawable)->pScreen->devPrivate))->devKind) >> 2; \
    } else { \
	(pointer) = (unsigned long *) \
		(((PixmapPtr)pDrawable)->devPrivate.ptr); \
	(width) = (int)(((PixmapPtr)pDrawable)->devKind) >> 2; \
    } \
}

#define I_H do{
#define I_T }while(0);
#define IMPORTANT_START I_H I_H I_H I_H I_H I_H I_H I_H I_H I_H
#define IMPORTANT_END	I_T I_T I_T I_T I_T I_T I_T I_T I_T I_T

#define OUTCODES_IMPLICIT(result, x, y) \
    if (x < bx1) \
	result |= OUT_LEFT; \
    if (x >= bx2) \
	result |= OUT_RIGHT; \
    if (y < by1) \
	result |= OUT_ABOVE; \
    if (y >= by2) \
	result |= OUT_BELOW;

#define OUTCODES(result, x, y, box) \
    if (x < box->x1) \
	result |= OUT_LEFT; \
    if (x >= box->x2) \
	result |= OUT_RIGHT; \
    if (y < box->y1) \
	result |= OUT_ABOVE; \
    if (y >= box->y2) \
	result |= OUT_BELOW;

#ifdef sun
#define WIDTH_FAST  1152
#endif

#ifdef ultrix
#define WIDTH_FAST  1024
#endif

#ifdef WIDTH_SHIFT
# define FUNC_NAME(e)	RROP_NAME(RROP_NAME_CAT(e,Shift))

# ifdef WIDTH_FAST
#  if WIDTH_FAST == 1024
#   define WIDTH_MUL(y,w)	((y) << 10)
#  endif

#  if WIDTH_FAST == 1152
#   define WIDTH_MUL(y,w)	(((y) << 10) + ((y) << 7))
#  endif

#  if WIDTH_FAST == 1280
#   define WIDTH_MUL(y,w)	(((y) << 10) + ((y) << 8))
#  endif

#  if WIDTH_FAST == 2048
#   define WIDTH_MUL(y,w)	((y) << 11)
#  endif
#  define NWIDTH(nwidth)    WIDTH_FAST
# endif

#else
# define FUNC_NAME(e)	RROP_NAME(e)
# define WIDTH_MUL(y,w)	((y) * (w))
# define NWIDTH(nwidth)	(nwidth)
#endif

#ifdef WIDTH_MUL

#if RROP == GXcopy
# ifndef WIDTH_FAST
#  define INCLUDE_OTHERS
# else
#  ifdef WIDTH_SHIFT
#   define INCLUDE_OTHERS
#  endif
# endif
#endif

int
#ifdef POLYSEGMENT
FUNC_NAME(cfb8SegmentSS1Rect) (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSegInit;
#else
FUNC_NAME(cfb8LineSS1Rect) (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int	mode;		/* Origin or Previous */
    int	npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    int		    bx1, bx2, by1, by2;
    int		    x1, y1;
    int		    x2, y2;
    int		    xorg, yorg;
    char	    *addr;
    int		    nwidth;
    cfbPrivGCPtr    devPriv;
    BoxPtr	    extents;
#ifdef POLYSEGMENT
    int		    capStyle;
#endif
    int		    oc1;
    STUPID int	    oc2;
#ifndef REARRANGE
    char	    *addrb;
    int		    e3, len;
    int		    stepx, stepy;
    RROP_DECLARE

#define adx x1
#define ady y1

#define e adx
#define e1 ady

#endif

#ifndef POLYSEGMENT
    DDXPointPtr	    ppt;
#else
    xSegment	    *pSeg;
#endif

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    cfbGetByteWidthAndPointer (pDrawable, nwidth, addr);
#ifndef REARRANGE
    RROP_FETCH_GCPRIV(devPriv);
#endif
    extents = &devPriv->pCompositeClip->extents;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    bx1 = extents->x1 - xorg;
    by1 = extents->y1 - yorg;
    bx2 = extents->x2 - xorg;
    by2 = extents->y2 - yorg;
    addr = addr + WIDTH_MUL(yorg,nwidth) + xorg;
#ifdef POLYSEGMENT
    capStyle = pGC->capStyle - CapNotLast;
    pSeg = pSegInit;
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x;
    y2 = ppt->y;
    oc2 = 0;
    OUTCODES_IMPLICIT (oc2, x2, y2);
#ifndef WIDTH_SHIFT
    mode -= CoordModePrevious;
#endif
    while (--npt)
#endif
    {
#ifdef POLYSEGMENT
	x1 = pSeg->x1;
	y1 = pSeg->y1;
	x2 = pSeg->x2;
	y2 = pSeg->y2;
	pSeg++;
	oc1 = 0;
	OUTCODES_IMPLICIT (oc1, x1, y1);
#else
	x1 = x2;
	y1 = y2;
	oc1 = oc2;
	++ppt;
	x2 = ppt->x;
	y2 = ppt->y;
#ifndef WIDTH_SHIFT
	if (!mode)
	{
	    x2 += x1;
	    y2 += y1;
	}
#endif
#endif
	oc2 = 0;
	OUTCODES_IMPLICIT (oc2, x2, y2);
	if (oc2 | oc1)
	{
	    if (oc1 & oc2)
		continue;

#ifdef POLYSEGMENT
	    break;
#else
#ifndef WIDTH_SHIFT
	    if (!mode)
	    {
		ppt[-1].x = x1;
		ppt[-1].y = y1;
		ppt[0].x = x2;
		ppt[0].y = y2;
	    }
#endif
	    break;
#endif
	}

	/* compute now to avoid needing x1, y1 later */
#ifndef REARRANGE
	addrb = addr + WIDTH_MUL(y1, nwidth) + x1;
#endif

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
	stepy = NWIDTH(nwidth);
	if ((ady = y2 - y1) <= 0)
	{
	    ady = -ady;
	    stepy = -stepy;
	}
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
	/* careful of order here - variables may be aliased */
	e1 = ady << 1;
	len = adx - 1;
	e = - adx;
	e3 = e << 1;

#ifdef POLYSEGMENT
	if (capStyle)
	    len++;
#endif

#ifdef REARRANGE
	{
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
	addrb = addr + WIDTH_MUL(y1, nwidth) + x1;
#endif

#ifdef NOTDEF
#ifdef LARGE_INSTRUCTION_CACHE
	if (!e1)
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

#if RROP == GXcopy
# define UNROLL	4
#else
# define UNROLL	4
#endif
	    while ((len -= UNROLL) >= 0)
	    {
		body body body body
#if UNROLL >= 8
		body body body body
#endif
#if UNROLL >= 12
		body body body body
#endif
#if UNROLL >= 16
		body body body body
#endif
	    }
	    switch (len)
	    {
	    case   -1: body case  -2: body case  -3: body
#if UNROLL >= 8
	    case   -4: body case  -5: body case  -6: body case -7: body
#endif
#if UNROLL >= 12
	    case   -8: body case  -9: body case -10: body case -11: body
#endif
#if UNROLL >= 16
	    case  -12: body case -13: body case -14: body case -15: body
#endif
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
#ifdef POLYSEGMENT
    if (nseg >= 0)
	return pSeg - pSegInit;
#else
    if (npt)
	return ppt - pptInit;
#endif

#ifndef POLYSEGMENT
    if (!pGC->capStyle == CapNotLast && !oc2 &&
	(x2 != pptInit->x || y2 != pptInit->y))
    {
#ifdef REARRANGE
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
#endif
	addrb = addr + WIDTH_MUL (y2, nwidth) + x2;
	RROP_SOLID (addrb);
    }
#endif
    return -1;
}

#ifndef REARRANGE
#undef adx
#undef ady
#undef e
#undef e1
#endif

#endif /* WIDTH_MUL */


#ifdef INCLUDE_OTHERS

extern void cfb8ClippedLineCopy(), cfb8ClippedLineXor(), cfb8ClippedLineGeneral(); 

#ifdef POLYSEGMENT

extern int cfb8SegmentSS1RectCopy(), cfb8SegmentSS1RectXor(), cfb8SegmentSS1RectGeneral(); 
#ifdef WIDTH_FAST
extern int cfb8SegmentSS1RectShiftCopy();
#endif

void
cfb8SegmentSS1Rect (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nseg;
    xSegment	    *pSegInit;
{
    int	    (*func)();
    void    (*clip)();
    int	    drawn;
    cfbPrivGCPtr    devPriv;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8SegmentSS1RectCopy;
	clip = cfb8ClippedLineCopy;
#ifdef WIDTH_FAST
	if (cfbGetByteWidth (pDrawable) == WIDTH_FAST)
	    func = cfb8SegmentSS1RectShiftCopy;
#endif
	break;
    case GXxor:
	func = cfb8SegmentSS1RectXor;
	clip = cfb8ClippedLineXor;
	break;
    default:
	func = cfb8SegmentSS1RectGeneral;
	clip = cfb8ClippedLineGeneral;
	break;
    }
    while (nseg)
    {
	drawn = (*func) (pDrawable, pGC, nseg, pSegInit);
	if (drawn == -1)
	    break;
	(*clip) (pDrawable, pGC,
			 pSegInit[drawn-1].x1, pSegInit[drawn-1].y1,
			 pSegInit[drawn-1].x2, pSegInit[drawn-1].y2,
			 &devPriv->pCompositeClip->extents,
			 pGC->capStyle == CapNotLast);
	pSegInit += drawn;
	nseg -= drawn;
    }
}

#else

extern int cfb8LineSS1RectCopy(), cfb8LineSS1RectXor(), cfb8LineSS1RectGeneral(); 
#ifdef WIDTH_FAST
extern int cfb8LineSS1RectShiftCopy();
#endif

void
cfb8LineSS1Rect (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pptInit;
{
    int	    (*func)();
    void    (*clip)();
    int	    drawn;
    cfbPrivGCPtr    devPriv;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8LineSS1RectCopy;
	clip = cfb8ClippedLineCopy;
#ifdef WIDTH_FAST
	if (mode != CoordModePrevious && cfbGetByteWidth (pDrawable) == WIDTH_FAST)
	    func = cfb8LineSS1RectShiftCopy;
#endif
	break;
    case GXxor:
	func = cfb8LineSS1RectXor;
	clip = cfb8ClippedLineXor;
	break;
    default:
	func = cfb8LineSS1RectGeneral;
	clip = cfb8ClippedLineGeneral;
	break;
    }
    while (npt)
    {
	drawn = (*func) (pDrawable, pGC, mode, npt, pptInit);
	if (drawn == -1)
	    break;
	(*clip) (pDrawable, pGC,
			 pptInit[drawn-1].x, pptInit[drawn-1].y,
			 pptInit[drawn].x, pptInit[drawn].y,
			 &devPriv->pCompositeClip->extents,
			 drawn != npt - 1 || pGC->capStyle == CapNotLast);
	pptInit += drawn;
	npt -= drawn;
    }
}

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )
#define ceiling(m,n)  (((m)-1)/(n) + 1)
#define SignTimes(sign,n)   (((sign) < 0) ? -(n) : (n))

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

#endif

#endif

#ifndef POLYSEGMENT
#ifndef WIDTH_SHIFT

void
RROP_NAME (cfb8ClippedLine) (pDrawable, pGC, x1, y1, x2, y2, boxp, shorten)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		x1, y1, x2, y2;
    BoxPtr	boxp;
    Bool	shorten;
{
    int		    oc1, oc2;
    int		    signdx, signdy, axis, e, e1, e3, len;
    int		    adx, ady;

    char	    *addr;
    int		    nwidth;
    int		    stepx, stepy;
    int		    xorg, yorg;


    cfbGetByteWidthAndPointer(pDrawable, nwidth, addr);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    x1 += xorg;
    y1 += yorg;
    x2 += xorg;
    y2 += yorg;

    signdx = 1;
    stepx = 1;
    if ((adx = x2 - x1) < 0)
    {
	adx = -adx;
	signdx = -1;
	stepx = -1;
    }
    signdy = 1;
    stepy = nwidth;
    if ((ady = y2 - y1) < 0)
    {
	ady = -ady;
	signdy = -1;
	stepy = -nwidth;
    }
    axis = X_AXIS;
    if (adx <= ady)
    {
	int	t;

	t = adx;
	adx = ady;
	ady = t;

	t = stepx;
	stepx = stepy;
	stepy = t;
	
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
	x1 = xt;
	y1 = yt;
    }
    if (oc1 | oc2)
	return;

    {
    register char	*addrb;
    RROP_DECLARE

    RROP_FETCH_GC(pGC);

    addrb = addr + (y1 * nwidth) + x1;

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

    }
}
#endif

#endif
