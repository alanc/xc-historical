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
/* $XConsortium: cfbbresd.c,v 1.8 91/03/11 14:58:04 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"

/* Dashed bresenham line */

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
    register unsigned char *addrb;
    register int e3 = e2-e1;
    int dashIndex;
    int dashOffset;
    int dashRemaining;
    unsigned long   xorFg, andFg, xorBg, andBg;
    Bool isCopy;
    int thisDash;

    dashOffset = *pdashOffset;
    dashIndex = *pdashIndex;
    dashRemaining = pDash[dashIndex] - dashOffset;
    isCopy = (rrops[0].rop == GXcopy && rrops[1].rop == GXcopy);
    xorFg = rrops[0].xor;
    andFg = rrops[0].and;
    xorBg = rrops[1].xor;
    andBg = rrops[1].and;
    if ((thisDash = dashRemaining) > len)
    {
	thisDash = len;
	dashRemaining -= len;
    }
#if (PPW == 4)
    /* point to first point */
    nlwidth <<= 2;
    addrb = (unsigned char *)(addrl) + (y1 * nlwidth) + x1;
    signdy *= nlwidth;
    if (axis == Y_AXIS)
    {
	int t;

	t = signdx;
	signdx = signdy;
	signdy = t;
    }
    e = e-e1;			/* to make looping easier */

#define BresStep(minor,major) {if ((e += e1) >= 0) { e += e3; minor; } major;}
#define Loop(store) while (thisDash--) {\
			store; \
 			BresStep(addrb+=signdy,addrb+=signdx) \
		    }

#define NextDash {\
    dashIndex++; \
    if (dashIndex == numInDashList) \
	dashIndex = 0; \
    dashRemaining = pDash[dashIndex]; \
    if ((thisDash = dashRemaining) > len) \
    { \
	dashRemaining -= len; \
	thisDash = len; \
    } \
}

    if (isCopy)
    {
	for (;;)
	{ 
	    len -= thisDash;
	    if (dashIndex & 1) {
		if (isDoubleDash) {
		    Loop(*addrb = xorBg)
		} else {
		    Loop(;)
		}
	    } else {
		Loop(*addrb = xorFg)
	    }
	    if (!len)
		break;
	    NextDash
	}
    }
    else
    {
	for (;;)
	{ 
	    len -= thisDash;
	    if (dashIndex & 1) {
		if (isDoubleDash) {
		    Loop(*addrb = DoRRop(*addrb,andBg, xorBg))
		} else {
		    Loop(;)
		}
	    } else {
		Loop(*addrb = DoRRop(*addrb,andFg, xorFg))
	    }
	    if (!len)
		break;
	    NextDash
	}
    }
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
