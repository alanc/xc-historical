/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: cfbbresd.c,v 1.6 89/11/08 17:12:51 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"

/* Dashed bresenham line */

#define StepDash\
    if (!--dashRemaining) { \
	if (++ dashIndex == numInDashList) \
	    dashIndex = 0; \
	dashRemaining = pDash[dashIndex]; \
	if (isDoubleDash) { \
	    xor = rrops[dashIndex & 1].xor; \
	    and = rrops[dashIndex & 1].and; \
	} \
	else \
	    dontdraw = (dashIndex & 1); \
    }
cfbBresD(rrops,
	 pdashIndex, pDash, numInDashList, pdashOffset, isDoubleDash,
	 addrl, nlwidth,
	 signdx, signdy, axis, x1, y1, e, e1, e2, len)
cfbRRopPtr  rrops;
int *pdashIndex;	/* current dash */
unsigned char *pDash;	/* dash list */
int numInDashList;	/* total length of dash list */
int *pdashOffset;	/* offset into current dash */
int isDoubleDash;
int *addrl;		/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int signdx, signdy;	/* signs of directions */
int axis;		/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;		/* initial point */
register int e;		/* error accumulator */
register int e1;	/* bresenham increments */
int e2;
int len;		/* length of line */
{
    register int yinc;	/* increment to next scanline, in bytes */
    register unsigned char *addrb;
    register int e3 = e2-e1;
    int dashIndex;
    int dashOffset;
    int dashRemaining;
    unsigned long   xor, and;
    int dontdraw;
    Bool isCopy;

    dashOffset = *pdashOffset;
    dashIndex = *pdashIndex;
    dashRemaining = pDash[dashIndex] - dashOffset;
    isCopy = (rrops[0].rop == GXcopy && rrops[1].rop == GXcopy);
    xor = rrops[0].xor;
    and = rrops[0].and;
    if (isDoubleDash)
    {
	if (dashIndex & 1)
	{
	    xor = rrops[1].xor;
	    and = rrops[1].and;
	}
	dontdraw = 0;
    }
    else
	dontdraw = (dashIndex & 1);
#if (PPW == 4)
    /* point to first point */
    nlwidth <<= 2;
    addrb = (unsigned char *)(addrl) + (y1 * nlwidth) + x1;
    yinc = signdy * nlwidth;
    e = e-e1;			/* to make looping easier */

    if (axis == X_AXIS)
    {
	if (isCopy)
	{
	    while(len--)
	    { 
		if (!dontdraw)
		    *addrb = xor;
		e += e1;
		if (e >= 0)
		{
		    addrb += yinc;
		    e += e3;
		}
		addrb += signdx;
		StepDash
	    }
	}
	else
	{
	    while(len--)
	    { 
		if (!dontdraw)
		    *addrb = DoRRop (*addrb, and, xor);
		e += e1;
		if (e >= 0)
		{
		    addrb += yinc;
		    e += e3;
		}
		addrb += signdx;
		StepDash
	    }
	}
    } /* if X_AXIS */
    else
    {
	if (isCopy)
	{
	    while(len--)
	    {
		if (!dontdraw)
		    *addrb = xor;
		e += e1;
		if (e >= 0)
		{
		    addrb += signdx;
		    e += e3;
		}
		addrb += yinc;
		StepDash
	    }
	}
	else
	{
	    while(len--)
	    {
		if (!dontdraw)
		    *addrb = DoRRop (*addrb, and, xor);
		e += e1;
		if (e >= 0)
		{
		    addrb += signdx;
		    e += e3;
		}
		addrb += yinc;
		StepDash
	    }
	}
    } /* else Y_AXIS */
#else
    {
    	register unsigned long   tmp;
	unsigned long leftbit, rightbit, bit;

    	/* point to longword containing first point */
    	addrl = (addrl + (y1 * nlwidth) + (x1 >> PWSH));
    	yinc = signdy * nlwidth;
    	e = e-e1;			/* to make looping easier */

    	leftbit = cfbmask[0] & planemask;
    	rightbit = cfbmask[PPW-1] & planemask;
    	bit = cfbmask[x1 & PIM] & planemask;

    	if (!bit)
	    return;			/* in case planemask == 0 */
    
    	if (axis == X_AXIS)
    	{
    	    if (signdx > 0)
    	    {
	    	while (len--)
	    	{ 
		    if (!dontdraw)
			DoMaskRRop (*addrl, and, xor, bit);
	    	    bit = SCRRIGHT(bit,1);
	    	    e += e1;
	    	    if (e >= 0)
	    	    {
		    	addrl += yinc;
		    	e += e3;
	    	    }
	    	    if (!bit)
	    	    {
		    	bit = leftbit;
		    	addrl++;
	    	    }
		    StepDash
	    	}
    	    }
    	    else
    	    {
	    	while (len--)
	    	{ 
		    if (!dontdraw)
			DoMaskRRop (*addrl, and, xor, bit);
	    	    e += e1;
	    	    bit = SCRLEFT(bit,1);
	    	    if (e >= 0)
	    	    {
		    	addrl += yinc;
		    	e += e3;
	    	    }
	    	    if (!bit)
	    	    {
		    	bit = rightbit;
		    	addrl--;
	    	    }
		    StepDash
	    	}
    	    }
    	} /* if X_AXIS */
    	else
    	{
    	    if (signdx > 0)
    	    {
	    	while(len--)
	    	{
		    if (!dontdraw)
			DoMaskRRop (*addrl, and, xor, bit);
	    	    e += e1;
	    	    if (e >= 0)
	    	    {
		    	bit = SCRRIGHT(bit,1);
		    	if (!bit)
 		    	{
			    bit = leftbit;
			    addrl++;
		    	}
		    	e += e3;
	    	    }
	    	    addrl += yinc;
		    StepDash
	    	}
    	    }
    	    else
    	    {
	    	while(len--)
	    	{
		    if (!dontdraw)
			DoMaskRRop (*addrl, and, xor, bit);
	    	    e += e1;
	    	    if (e >= 0)
	    	    {
		    	bit = SCRLEFT(bit,1);
		    	if (!bit)
 		    	{
			    bit = rightbit;
			    addrl--;
		    	}
		    	e += e3;
	    	    }
	    	    addrl += yinc;
		    StepDash
	    	}
    	    }
    	} /* else Y_AXIS */
    }
#endif
    *pdashIndex = dashIndex;
    *pdashOffset = pDash[dashIndex] - dashRemaining;
}
