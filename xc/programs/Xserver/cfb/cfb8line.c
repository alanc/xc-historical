/*
 * $XConsortium: cfb8line.c,v 1.29 94/07/28 14:38:16 dpw Exp $
 *
Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
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
#include "miline.h"

#ifdef PIXEL_ADDR

#if defined(__GNUC__) && defined(mc68020)
#define STUPID volatile
#define REARRANGE
#else
#define STUPID
#endif

#ifdef __GNUC__
/* lame compiler doesn't even look at 'register' attributes */
#define I_H do{
#define I_T }while(0);
#define IMPORTANT_START I_H I_H I_H I_H I_H I_H I_H I_H I_H I_H
#define IMPORTANT_END	I_T I_T I_T I_T I_T I_T I_T I_T I_T I_T
#else
#define IMPORTANT_START
#define IMPORTANT_END
#endif

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

#ifdef POLYSEGMENT

# ifdef sun
#  define WIDTH_FAST  1152
# endif

# ifdef ultrix
#  define WIDTH_FAST  1024
# endif

# ifdef Mips
#  define WIDTH_FAST 4096
# endif
# ifdef WIDTH_FAST
#  if WIDTH_FAST == 1024
#   define FAST_MUL(y)	((y) << 10)
#  endif

#  if WIDTH_FAST == 1152
#   define FAST_MUL(y)	(((y) << 10) + ((y) << 7))
#  endif

#  if WIDTH_FAST == 1280
#   define FAST_MUL(y)	(((y) << 10) + ((y) << 8))
#  endif

#  if WIDTH_FAST == 2048
#   define FAST_MUL(y)	((y) << 11)
#  endif

#  if WIDTH_FAST == 4096
#   define FAST_MUL(y)	((y) << 12)
#  endif
# endif

# if defined(WIDTH_SHIFT)
#  ifdef FAST_MUL
#   define FUNC_NAME(e)	    RROP_NAME(RROP_NAME_CAT(e,Shift))
#   if RROP == GXcopy
#    define INCLUDE_OTHERS
#    define SERIOUS_UNROLLING
#   endif
#   define INCLUDE_DRAW
#   define NWIDTH(nwidth)   WIDTH_FAST
#   define WIDTH_MUL(y,w)   FAST_MUL(y)
#  endif
# else
#  define FUNC_NAME(e)	    RROP_NAME(e)
#  define WIDTH_MUL(y,w)    ((y) * (w))
#  define NWIDTH(nwidth)    (nwidth)
#  define INCLUDE_DRAW
#  if !defined (FAST_MUL) && RROP == GXcopy
#   define INCLUDE_OTHERS
#   define SERIOUS_UNROLLING
#  endif
# endif
#else

# define INCLUDE_DRAW
# define WIDTH_MUL(y,w)	((y) * (w))
# define NWIDTH(nwidth)	nwidth
# ifdef PREVIOUS
#  define FUNC_NAME(e)	RROP_NAME(RROP_NAME_CAT(e,Previous))
# else
#  define FUNC_NAME(e)	RROP_NAME(e)
#  if RROP == GXcopy
#   define INCLUDE_OTHERS
#   ifdef PLENTIFUL_REGISTERS
#    define SAVE_X2Y2
#   endif
#   define ORIGIN
#   define SERIOUS_UNROLLING
#  else
#   define EITHER_MODE
#  endif
# endif
#endif

#ifdef INCLUDE_DRAW

int
#ifdef POLYSEGMENT
FUNC_NAME(cfb8SegmentSS1Rect) (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSegInit;
#else
FUNC_NAME(cfb8LineSS1Rect) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			    x1p,y1p,x2p,y2p)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int	mode;		/* Origin or Previous */
    int	npt;		/* number of points */
    DDXPointPtr pptInit, pptInitOrig;
    int	*x1p, *y1p, *x2p, *y2p;
#endif /* POLYSEGEMENT */
{
    register long   e;
    register int    y1_or_e1;
    register PixelType   *addrp;
    register int    stepmajor;
    register int    stepminor;
#ifndef REARRANGE
    register long   e3;
#endif
#ifdef mc68000
    register short  x1_or_len;
#else
    register int    x1_or_len;
#endif
    RROP_DECLARE

#ifdef SAVE_X2Y2
# define c2 y2
#else
    register int    c2;
#endif
#ifndef ORIGIN
    register int _x1, _y1, _x2, _y2;	/* only used for CoordModePrevious */
    int extents_x1, extents_y1, extents_x2, extents_y2;
#endif /* !ORIGIN */
#ifndef PREVIOUS
    register int upperleft, lowerright;
    CARD32	 ClipMask = 0x80008000;
#endif /* !PREVIOUS */
#ifdef POLYSEGMENT
    register int    capStyle;
#endif /* POLYSEGMENT */
#ifdef SAVE_X2Y2
    register int    x2, y2;
# define X1  x1_or_len
# define Y1  y1_or_e1
# define X2  x2
# define Y2  y2
#else
# ifdef POLYSEGMENT
#  define X1  x1_or_len
#  define Y1  y1_or_e1
# else
#  define X1  intToX(y1_or_e1)
#  define Y1  intToY(y1_or_e1)
# endif /* POLYSEGMENT */
# define X2  intToX(c2)
# define Y2  intToY(c2)
#endif /* SAVE_X2Y2 */
    PixelType   *addr;
    int		    nwidth;
    cfbPrivGCPtr    devPriv;
    BoxPtr	    extents;
    int		    *ppt;
    int		    octant;
    unsigned int    bias = miGetZeroLineBias(pDrawable->pScreen);

    devPriv = cfbGetGCPrivate(pGC);
    cfbGetPixelWidthAndPointer (pDrawable, nwidth, addr);
#ifndef REARRANGE
    RROP_FETCH_GCPRIV(devPriv);
#endif
    extents = &devPriv->pCompositeClip->extents;
#ifndef PREVIOUS
    c2 = *((int *) &pDrawable->x);
    c2 -= (c2 & 0x8000) << 1;
    upperleft = *((int *) &extents->x1) - c2;
    lowerright = *((int *) &extents->x2) - c2 - 0x00010001;
#endif /* !PREVIOUS */
    addr = addr + WIDTH_MUL(pDrawable->y,nwidth) + pDrawable->x;
#ifdef POLYSEGMENT
    capStyle = pGC->capStyle - CapNotLast;
    ppt = (int *) pSegInit;
    while (nseg--)
#else /* POLYSEGMENT */
#ifdef EITHER_MODE
    mode -= CoordModePrevious;
    if (!mode)
#endif /* EITHER_MODE */	
#ifndef ORIGIN
    {	/* CoordModePrevious */
	ppt = (int *)pptInit + 1;
	_x1 = *x1p;
	_y1 = *y1p;
	extents_x1 = extents->x1 - pDrawable->x;
	extents_x2 = extents->x2 - pDrawable->x;
	extents_y1 = extents->y1 - pDrawable->y;
	extents_y2 = extents->y2 - pDrawable->y;
	if (_x1 < extents_x1 || _x1 >= extents_x2 ||
	    _y1 < extents_y1 || _y1 >= extents_y2)
	{
	    c2 = *ppt++;
	    intToCoord(c2, _x2, _y2);
	    *x2p = _x1 + _x2;
	    *y2p = _y1 + _y2;
	    return 1;
	}
	addrp = addr + WIDTH_MUL(_y1, nwidth) + _x1;
	_x2 = _x1;
	_y2 = _y1;	
    }
#endif /* !ORIGIN */
#ifdef EITHER_MODE
    else
#endif /* EITHER_MODE */
#ifndef PREVIOUS
    {
	ppt = (int *) pptInit;
	c2 = *ppt++;
	if (isClipped (c2, upperleft, lowerright))
	{
	    return 1;
	}
#ifdef SAVE_X2Y2
	intToCoord(c2,x2,y2);
#endif
	addrp = addr + WIDTH_MUL(Y2, nwidth) + X2;
    }
#endif /* !PREVIOUS */    
    while (--npt)
#endif /* POLYSEGMENT */
    {
#ifdef POLYSEGMENT
	y1_or_e1 = ppt[0];
	c2 = ppt[1];
	ppt += 2;
	if (isClipped(y1_or_e1,upperleft,lowerright)|isClipped(c2,upperleft,lowerright))
	    break;
	intToCoord(y1_or_e1,x1_or_len,y1_or_e1);
	/* compute now to avoid needing x1, y1 later */
	addrp = addr + WIDTH_MUL(y1_or_e1, nwidth) + x1_or_len;
#else /* !POLYSEGMENT */
#ifdef EITHER_MODE
	if (!mode)
#endif /* EITHER_MODE */	
#ifndef ORIGIN
	{	
	    /* CoordModePrevious */
	    _x1 = _x2;
	    _y1 = _y2;
	    c2 = *ppt++;
	    intToCoord(c2, _x2, _y2);
	    _x2 = _x1 + _x2;
	    _y2 = _y1 + _y2;

	    if (_x1 < extents_x1 || _x1 >= extents_x2 ||
		_y1 < extents_y1 || _y1 >= extents_y2)
	    {
		break;
	    }
	    CalcLineDeltas(_x1, _y1, _x2, _y2, x1_or_len, y1_or_e1,
			   stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	}
#endif /* !ORIGIN */
#ifdef EITHER_MODE
	else
#endif /* EITHER_MODE */
#ifndef PREVIOUS
        {
#ifndef SAVE_X2Y2
	    y1_or_e1 = c2;
#else
	    y1_or_e1 = y2;
	    x1_or_len = x2;
#endif /* SAVE_X2Y2 */
	    c2 = *ppt++;

	    if (isClipped (c2, upperleft, lowerright))
		break;
#ifdef SAVE_X2Y2
	    intToCoord(c2,x2,y2);
#endif
	    CalcLineDeltas(X1, Y1, X2, Y2, x1_or_len, y1_or_e1,
			   stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	}
#endif /* !PREVIOUS */
#endif /* POLYSEGMENT */

#ifdef POLYSEGMENT
	CalcLineDeltas(X1, Y1, X2, Y2, x1_or_len, y1_or_e1,
		       stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	/*
	 * although the horizontal code works for polyline, it
	 * slows down 10 pixel lines by 15%.  Thus, this
	 * code is optimized for horizontal segments and
	 * random orientation lines, which seems like a reasonable
	 * assumption
	 */
	if (y1_or_e1 != 0)
	{
#endif /* POLYSEGMENT */
	if (x1_or_len < y1_or_e1)
	{
#ifdef REARRANGE
	    register int	e3;
#endif

	    e3 = x1_or_len;
	    x1_or_len = y1_or_e1;
	    y1_or_e1 = e3;

	    e3 = stepminor;
	    stepminor = stepmajor;
	    stepmajor = e3;
	    SetYMajorOctant(octant);
	}

	e = -x1_or_len;
#ifdef POLYSEGMENT
	if (!capStyle)
	    x1_or_len--;
#endif

	{
#ifdef REARRANGE
	register int e3;
	RROP_DECLARE
	RROP_FETCH_GCPRIV(devPriv);
#endif

	y1_or_e1 = y1_or_e1 << 1;
	e3 = e << 1;

	FIXUP_ERROR(e, octant, bias);

#define body {\
	    RROP_SOLID(addrp); \
	    addrp += stepmajor; \
	    e += y1_or_e1; \
	    if (e >= 0) \
	    { \
		addrp += stepminor; \
		e += e3; \
	     } \
	}

#ifdef LARGE_INSTRUCTION_CACHE

# ifdef SERIOUS_UNROLLING
#  define UNROLL	16
# else
#  define UNROLL	4
# endif
#define CASE(n)	case -n: body

	while ((x1_or_len -= UNROLL) >= 0)
	{
	    body body body body
# if UNROLL >= 8
	    body body body body
# endif
# if UNROLL >= 12
	    body body body body
# endif
# if UNROLL >= 16
	    body body body body
# endif
	}
	switch (x1_or_len)
	{
	CASE(1) CASE(2) CASE(3)
# if UNROLL >= 8
	CASE(4) CASE(5) CASE(6) CASE(7)
# endif
# if UNROLL >= 12
	CASE(8) CASE(9) CASE(10) CASE(11)
# endif
# if UNROLL >= 16
	CASE(12) CASE(13) CASE(14) CASE(15)
# endif
	}
#else /* !LARGE_INSTRUCTION_CACHE */

	IMPORTANT_START
	IMPORTANT_START

	if (x1_or_len & 1)
	    body
	x1_or_len >>= 1;
	while (x1_or_len--) {
	    body body
	}

	IMPORTANT_END
	IMPORTANT_END
#endif /* LARGE_INSTRUCTION_CACHE */

#ifdef POLYSEGMENT
	RROP_SOLID(addrp);
#endif
	}
#undef body
#ifdef POLYSEGMENT
	}
	else
	{
# ifdef REARRANGE
	    register int    e3;
	    RROP_DECLARE
	    RROP_FETCH_GCPRIV(devPriv);
# endif /* REARRANGE */
	    if (stepmajor < 0)
	    {
		addrp -= x1_or_len;
		if (capStyle)
		    x1_or_len++;
		else
		    addrp++;
	    }
	    else
	    {
		if (capStyle)
		    x1_or_len++;
	    }
	    y1_or_e1 = ((int) addrp) & PIM;
	    addrp = (PixelType *) (((unsigned char *) addrp) - y1_or_e1);
#if PGSZ == 32
#  if PWSH != 2
	    y1_or_e1 >>= (2 - PWSH);
#  endif
#else /* PGSZ == 64 */
#  if PWSH != 3
	    y1_or_e1 >>= (3 - PWSH);
#  endif
#endif /* PGSZ */
	    if (y1_or_e1 + x1_or_len <= PPW)
	    {
		if (x1_or_len)
		{
		    maskpartialbits(y1_or_e1, x1_or_len, e)
		    RROP_SOLID_MASK((unsigned long *) addrp, e);
		}
	    }
	    else
	    {
	    	maskbits(y1_or_e1, x1_or_len, e, e3, x1_or_len)
	    	if (e)
	    	{
		    RROP_SOLID_MASK((unsigned long *) addrp, e);
		    addrp += PPW;
	    	}
		RROP_SPAN(addrp, x1_or_len)
	    	if (e3)
		    RROP_SOLID_MASK((unsigned long *) addrp, e3);
	    }
	}
#endif /* POLYSEGMENT */
    }
#ifdef POLYSEGMENT
    if (nseg >= 0)
	return (xSegment *) ppt - pSegInit;
#else
    if (npt)
    {
#ifdef EITHER_MODE
	if (!mode)
#endif /* EITHER_MODE */
#ifndef ORIGIN
	{
	    *x1p = _x1;
	    *y1p = _y1;		
	    *x2p = _x2;
	    *y2p = _y2;
	}
#endif /* !ORIGIN */	    
	return ((DDXPointPtr) ppt - pptInit) - 1;
    }
#endif /* POLYSEGMENT */

#ifndef POLYSEGMENT
# ifndef ORIGIN
#  define C2  c2
# else
#  define C2  ppt[-1]
# endif
#ifdef EITHER_MODE
    if (pGC->capStyle != CapNotLast &&
	((mode ? (C2 != *((int *) pptInitOrig))
	       : ((_x2 != pptInitOrig->x) ||
		  (_y2 != pptInitOrig->y)))
	 || (ppt == ((int *)pptInitOrig) + 2)))
#endif /* EITHER_MODE */
#ifdef PREVIOUS
    if (pGC->capStyle != CapNotLast &&
	((_x2 != pptInitOrig->x) ||
	 (_y2 != pptInitOrig->y) ||
	 (ppt == ((int *)pptInitOrig) + 2)))
#endif /* PREVIOUS */
#ifdef ORIGIN
    if (pGC->capStyle != CapNotLast &&
	((C2 != *((int *) pptInitOrig)) ||
	 (ppt == ((int *)pptInitOrig) + 2)))
#endif /* !PREVIOUS */
    {
# ifdef REARRANGE
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
# endif
	RROP_SOLID (addrp);
    }
#endif /* !POLYSEGMENT */
    return -1;
}

#endif /* INCLUDE_DRAW */


#ifdef INCLUDE_OTHERS

#ifdef POLYSEGMENT

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

    devPriv = cfbGetGCPrivate(pGC);
#ifdef NO_ONE_RECT
    if (REGION_NUM_RECTS(devPriv->pCompositeClip) != 1)
    {
       cfbSegmentSS(pDrawable, pGC, nseg, pSegInit);
       return;
    }
#endif
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8SegmentSS1RectCopy;
	clip = cfb8ClippedLineCopy;
#ifdef FAST_MUL
	if (cfbGetPixelWidth (pDrawable) == WIDTH_FAST)
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

#else /* POLYSEGMENT */

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
    int x1, y1, x2, y2;
    DDXPointPtr pptInitOrig = pptInit;

    devPriv = cfbGetGCPrivate(pGC);
#ifdef NO_ONE_RECT
    if (REGION_NUM_RECTS(devPriv->pCompositeClip) != 1)
    {
       cfbLineSS(pDrawable, pGC, mode, npt, pptInit);
       return;
    }
#endif
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8LineSS1RectCopy;
	clip = cfb8ClippedLineCopy;
	if (mode == CoordModePrevious)
	    func = cfb8LineSS1RectPreviousCopy;
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
    if (mode == CoordModePrevious)
    {
	x1 = pptInit->x;
	y1 = pptInit->y;
	while (npt > 1)
	{
	    drawn = (*func) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			     &x1, &y1, &x2, &y2);
	    if (drawn == -1)
		break;
	    (*clip) (pDrawable, pGC, x1, y1, x2, y2,
		     &devPriv->pCompositeClip->extents,
		     drawn != npt - 1 || pGC->capStyle == CapNotLast);
	    pptInit += drawn;
	    npt -= drawn;
	    x1 = x2;
	    y1 = y2;
	}
    }
    else
    {
	while (npt > 1)
	{
	    drawn = (*func) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			     &x1, &y1, &x2, &y2);
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
}

#endif /* else POLYSEGMENT */
#endif /* INCLUDE_OTHERS */

#if !defined(POLYSEGMENT) && !defined (PREVIOUS)

void
RROP_NAME (cfb8ClippedLine) (pDrawable, pGC, x1, y1, x2, y2, boxp, shorten)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		x1, y1, x2, y2;
    BoxPtr	boxp;
    Bool	shorten;
{
    int		    oc1, oc2;
    int		    e, e1, e3, len;
    int		    adx, ady;

    PixelType	    *addr;
    int		    nwidth;
    int		    stepx, stepy;
    int		    xorg, yorg;
    int             new_x1, new_y1, new_x2, new_y2;
    Bool	    pt1_clipped, pt2_clipped;
    int		    changex, changey, result;
    int		    octant;
    unsigned int    bias = miGetZeroLineBias(pDrawable->pScreen);

    cfbGetPixelWidthAndPointer(pDrawable, nwidth, addr);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    x1 += xorg;
    y1 += yorg;
    x2 += xorg;
    y2 += yorg;
    oc1 = 0;
    oc2 = 0;
    OUTCODES (oc1, x1, y1, boxp);
    OUTCODES (oc2, x2, y2, boxp);

    if (oc1 & oc2)
	return;

    CalcLineDeltas(x1, y1, x2, y2, adx, ady, stepx, stepy, 1, nwidth, octant);

    if (adx <= ady)
    {
	int	t;

	t = adx;
	adx = ady;
	ady = t;

	t = stepx;
	stepx = stepy;
	stepy = t;
	
	SetYMajorOctant(octant);
    }
    e = - adx;
    e1 = ady << 1;
    e3 = - (adx << 1);

    FIXUP_ERROR(e, octant, bias);

    new_x1 = x1;
    new_y1 = y1;
    new_x2 = x2;
    new_y2 = y2;
    pt1_clipped = 0;
    pt2_clipped = 0;

    if (IsXMajorOctant(octant))
    {
	result = miZeroClipLine(boxp->x1, boxp->y1, boxp->x2 - 1, boxp->y2 - 1,
				&new_x1, &new_y1, &new_x2, &new_y2,
				adx, ady,
				&pt1_clipped, &pt2_clipped,
				octant, bias, oc1, oc2);
	if (result == -1)
	    return;
	
	len = abs(new_x2 - new_x1) - 1; /* this routine needs the "-1" */
	
	/* if we've clipped the endpoint, always draw the full length
	 * of the segment, because then the capstyle doesn't matter 
	 * if x2,y2 isn't clipped, use the capstyle
	 * (shorten == TRUE <--> CapNotLast)
	 */
	if (pt2_clipped || !shorten)
	    len++;
	
	if (pt1_clipped)
	{
	    /* must calculate new error terms */
	    changex = abs(new_x1 - x1);
	    changey = abs(new_y1 - y1);
	    e = e + changey * e3 + changex * e1;	    
	}
    }
    else /* Y_AXIS */
    {
	result = miZeroClipLine(boxp->x1, boxp->y1, boxp->x2 - 1, boxp->y2 - 1,
				&new_x1, &new_y1, &new_x2, &new_y2,
				ady, adx,
				&pt1_clipped, &pt2_clipped,
				octant, bias, oc1, oc2);
	if (result == -1)
	    return;
	
	len = abs(new_y2 - new_y1) - 1; /* this routine needs the "-1" */
	
	/* if we've clipped the endpoint, always draw the full length
	 * of the segment, because then the capstyle doesn't matter 
	 * if x2,y2 isn't clipped, use the capstyle
	 * (shorten == TRUE <--> CapNotLast)
	 */
	if (pt2_clipped || !shorten)
	    len++;
	
	if (pt1_clipped)
	{
	    /* must calculate new error terms */
	    changex = abs(new_x1 - x1);
	    changey = abs(new_y1 - y1);
	    e = e + changex * e3 + changey * e1;
	}
    }
    x1 = new_x1;
    y1 = new_y1;
    {
    register PixelType	*addrp;
    RROP_DECLARE

    RROP_FETCH_GC(pGC);

    addrp = addr + (y1 * nwidth) + x1;

#ifndef REARRANGE
    if (!ady)
    {
#define body	{ RROP_SOLID(addrp); addrp += stepx; }
	while (len >= PGSZB)
	{
	    body body body body
#if PGSZ == 64
	    body body body body
#endif
	    len -= PGSZB;
	}
	switch (len)
	{
#if PGSZ == 64
	case  7: body case 6: body case 5: body case 4: body
#endif
	case  3: body case 2: body case 1: body
	}
#undef body
    }
    else
#endif /* !REARRANGE */
    {
#define body {\
	    RROP_SOLID(addrp); \
	    addrp += stepx; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrp += stepy; \
		e += e3; \
	     } \
	}

#ifdef LARGE_INSTRUCTION_CACHE
	while ((len -= PGSZB) >= 0)
	{
	    body body body body
#if PGSZ == 64
	    body body body body
#endif
	}
	switch (len)
	{
	case  -1: body case -2: body case -3: body
#if PGSZ == 64
	case  -4: body case -5: body case -6: body case -7: body
#endif
	}
#else /* !LARGE_INSTRUCTION_CACHE */
	IMPORTANT_START;

	while ((len -= 2) >= 0)
	{
	    body body;
	}
	if (len & 1)
	    body;

	IMPORTANT_END;
#endif /* LARGE_INSTRUCTION_CACHE */
    }
    RROP_SOLID(addrp);
#undef body
    }
}

#endif /* !POLYSEGMENT && !PREVIOUS */
#endif /* PIXEL_ADDR */
