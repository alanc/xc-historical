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
/* $XConsortium: cfbbres.c,v 1.4 89/09/19 15:34:59 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

#ifdef LARGE_INSTRUCTION_CACHE
#define Duff(count,body) \
    switch (count & 15) { \
    case 15: body    case 14: body    case 13: body    case 12: body\
    case 11: body    case 10: body    case 9: body    case 8: body\
    case 7: body    case 6: body    case 5: body    case 4: body\
    case 3: body    case 2: body    case 1: body \
    } \
    while ((count -= 16) >= 0) { \
	body body body body body body body body \
	body body body body body body body body \
    }
#else /* LARGE_INSTRUCTION_CACHE */
#define Duff(count,body) \
    switch (count & 3) { \
    case 3: body    case 2: body    case 1: body \
    } \
    while ((count -= 4) >= 0) { \
	body body body body \
    }
#endif /* LARGE_INSTRUCTION_CACHE */

cfbBresS(rop, pixel, planemask, addrl, nlwidth, signdx, signdy, axis, x1, y1, e, e1, e2, len)
int rop;
register unsigned long pixel;
unsigned long planemask;
int *addrl;		/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
register int signdx;
int signdy;		/* signs of directions */
int axis;		/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;		/* initial point */
register int e;		/* error accumulator */
register int e1;	/* bresenham increments */
int e2;
int len;	/* length of line */
{
    register int yinc;	/* increment to next scanline, in bytes */
    register int xinc;	/* increment to next pixel */
    register unsigned char *addrb;		/* bitmask long pointer 
						 * cast to char pointer */

    register int e3 = e2-e1;

#if (PPW == 4)
    if (len == 0)
	return;
    if ((planemask & PIM) == PIM)
    {
    	/* point to first point */
    	nlwidth <<= 2;
    	addrb = (unsigned char *)(addrl) + (y1 * nlwidth) + x1;
	if (signdy < 0)
	    nlwidth = -nlwidth;
    	xinc = signdx;
    	e = e-e1;			/* to make looping easier */
    
    	if (axis == X_AXIS)
    	{
	    if (rop == GXcopy)
	    {
		len--;
	    	Duff (len,
	    	{ 
	    	    *addrb = pixel;
	    	    e += e1;
	    	    addrb += xinc;
	    	    if (e >= 0)
	    	    {
		    	addrb += nlwidth;
		    	e += e3;
	    	    }
	    	}
	    	)
		*addrb = pixel;
	    }
	    else
	    {
	    	while(len--)
	    	{ 
	    	    *addrb = DoRop (rop, pixel, *addrb);
	    	    e += e1;
	    	    addrb += xinc;
	    	    if (e >= 0)
	    	    {
		    	addrb += nlwidth;
		    	e += e3;
	    	    }
	    	}
	    }
    	} /* if X_AXIS */
    	else
    	{
	    if (rop == GXcopy)
	    {
		len--;
	    	Duff (len,
	    	{
	    	    *addrb = pixel;
	    	    e += e1;
	    	    addrb += nlwidth;
	    	    if (e >= 0)
	    	    {
		    	addrb += xinc;
		    	e += e3;
	    	    }
    	    	}
	    	)
		*addrb = pixel;
	    }
	    else
	    {
	    	while(len--)
	    	{
	    	    *addrb = DoRop (rop, pixel, *addrb);
	    	    e += e1;
	    	    addrb += nlwidth;
	    	    if (e >= 0)
	    	    {
		    	addrb += xinc;
		    	e += e3;
	    	    }
    	    	}
	    }
    	} /* else Y_AXIS */
    }
    else
#endif
    {
    	register unsigned long   tmp, bit, leftbit, rightbit;

    	/* point to longword containing first point */
    	addrl = (addrl + (y1 * nlwidth) + (x1 >> PWSH));
    	yinc = signdy * nlwidth;
    	e = e-e1;			/* to make looping easier */

    	planemask = PFILL(planemask);
    	pixel = PFILL(pixel);

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
		    tmp = *addrl;
	    	    *addrl = tmp & ~bit | DoRop (rop, pixel, tmp) & bit;
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
	    	}
    	    }
    	    else
    	    {
	    	while (len--)
	    	{ 
		    tmp = *addrl;
	    	    *addrl = tmp & ~bit | DoRop (rop, pixel, tmp) & bit;
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
	    	}
    	    }
    	} /* if X_AXIS */
    	else
    	{
    	    if (signdx > 0)
    	    {
	    	while(len--)
	    	{
		    tmp = *addrl;
	    	    *addrl = tmp & ~bit | DoRop (rop, pixel, tmp) & bit;
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
	    	}
    	    }
    	    else
    	    {
	    	while(len--)
	    	{
		    tmp = *addrl;
	    	    *addrl = tmp & ~bit | DoRop (rop, pixel, tmp) & bit;
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
	    	}
    	    }
    	} /* else Y_AXIS */
    } 
}
