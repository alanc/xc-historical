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
/* $XConsortium: mfbhrzvert.c,v 1.10 89/03/16 14:47:31 jim Exp $ */
#include "X.h"

#include "gc.h"
#include "window.h"
#include "pixmap.h"
#include "region.h"

#include "cfb.h"
#include "cfbmskbits.h"

/* horizontal solid line
   abs(len) > 1
*/
cfbHorzS(rop, pixel, addrl, nlwidth, x1, y1, len)
int rop;
unsigned long	pixel;
register int *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1;			/* initial point */ 
int y1;
int len;		/* length of line */
{
    register int startmask;
    register int endmask;
    register int nlmiddle;
    unsigned long   scrbits;

    pixel = PFILL (pixel);
    /* force the line to go left to right
       but don't draw the last point
    */
    if (len < 0)
    {
	x1 += len;
	x1 += 1;
	len = -len;
    }

    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);

    /* all bits inside same longword */
    if ( ((x1 & PIM) + len) < PPW)
    {
	maskpartialbits(x1, len, startmask);
	scrbits = *addrl;
	*addrl = (scrbits & ~startmask) |
		 (DoRop (rop, pixel, scrbits) & startmask);
    }
    else
    {
	maskbits(x1, len, startmask, endmask, nlmiddle);
	if (startmask)
	{
	    scrbits = *addrl;
	    *addrl++ = (scrbits & ~startmask) |
		       (DoRop (rop, pixel, scrbits) & startmask);
	}
	while (nlmiddle--)
	{
	    *addrl = DoRop (rop, pixel, *addrl);
	    ++addrl;
	}
	if (endmask)
	{
	    scrbits = *addrl;
   	    *addrl = (scrbits & ~endmask) |
		     (DoRop (rop, pixel, scrbits) & endmask);
	}
    }
}

/* vertical solid line
   this uses do loops because pcc (Ultrix 1.2, bsd 4.2) generates
   better code.  sigh.  we know that len will never be 0 or 1, so
   it's OK to use it.
*/

cfbVertS(rop, pixel, addrl, nlwidth, x1, y1, len)
int rop;
unsigned long pixel;
register int *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1, y1;		/* initial point */
register int len;	/* length of line */
{
#if (PPW == 4)
    register unsigned char    *bits = (unsigned char *) addrl;

    nlwidth <<= 2;
    bits = bits + (y1 * nlwidth) + x1;
    if (len < 0)
    {
	nlwidth = -nlwidth;
	len = -len;
    }

    /*
     * special case copy to avoid a test per pixel
     */
    if (rop == GXcopy)
    {
	while (len--)
 	{
	    *bits = pixel;
	    bits += nlwidth;
	}
    }
    else
    {
	while (len--)
 	{
	    *bits = DoRop(rop, pixel, *bits);
	    bits += nlwidth;
	}
    }
#else
    register unsigned long  scrbits, mask;

    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);

    if (len < 0)
    {
	nlwidth = -nlwidth;
	len = -len;
    }
 
    mask = cfbmask[x1 & PIM];
    pixel = PFILL (pixel);

    do
    {
	scrbits = *addrl;
	*addrl = (scrbits & ~ mask) | (DoRop (rop, pixel, scrbits) & mask);
	addrl += nlwidth;
    }
    while (--len);
#endif
}
