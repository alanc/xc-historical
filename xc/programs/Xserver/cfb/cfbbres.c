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
/* $XConsortium: cfbbres.c,v 1.2 89/09/01 15:46:31 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"

#if (PPW == 4)
#include "cfb8bit.h"
#endif

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

#undef Duff
#ifdef mips
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
#else
#define Duff(count,body) \
    switch (count & 3) { \
    case 3: body    case 2: body    case 1: body \
    } \
    while ((count -= 4) >= 0) { \
	body body body body \
    }
#endif

cfbBresS(rop, pixel, addrl, nlwidth, signdx, signdy, axis, x1, y1, e, e1, e2, len)
int rop;
register unsigned long pixel;
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
    /* point to first point */
    nlwidth <<= 2;
    addrb = (unsigned char *)(addrl) + (y1 * nlwidth) + x1;
    yinc = signdy * nlwidth;
    xinc = signdx;
    e = e-e1;			/* to make looping easier */

    if (axis == X_AXIS)
    {
	if (rop == GXcopy)
	{
	    Duff (len,
	    { 
	    	*addrb = pixel;
	    	e += e1;
	    	addrb += xinc;
	    	if (e >= 0)
	    	{
		    addrb += yinc;
		    e += e3;
	    	}
	    }
	    )
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
		    addrb += yinc;
		    e += e3;
	    	}
	    }
	}
    } /* if X_AXIS */
    else
    {
	if (rop == GXcopy)
	{
	    Duff (len,
	    {
	    	*addrb = pixel;
	    	e += e1;
	    	addrb += yinc;
	    	if (e >= 0)
	    	{
		    addrb += xinc;
		    e += e3;
	    	}
    	    }
	    )
	}
	else
	{
	    while(len--)
	    {
	    	*addrb = DoRop (rop, pixel, *addrb);
	    	e += e1;
	    	addrb += yinc;
	    	if (e >= 0)
	    	{
		    addrb += xinc;
		    e += e3;
	    	}
    	    }
	}
    } /* else Y_AXIS */
#else
/*
 * arbitrary pixel size case is harder...
 */
#endif
} 
