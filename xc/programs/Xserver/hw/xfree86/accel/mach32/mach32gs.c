/* $XConsortium$ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 1993 by Kevin E. Martin, Chapel Hill, North Carolina.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND KEVIN E. MARTIN AND RICKARD E. FAITH DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DIGITAL OR KEVIN E. MARTIN
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
Modified for the Mach-8 by Rickard E. Faith (faith@cs.unc.edu)
Modified for the Mach32 by Kevin E. Martin (martin@cs.unc.edu)

******************************************************************/


#include "X.h"
#include "Xmd.h"
#include "servermd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "mach32.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
void
mach32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pdstStart;	/* where to put the bits */
{
    int			j;
    unsigned char	*pdst; /* where to put the bits */
    int			pixmapStride;

    if ((pDrawable->type != DRAWABLE_WINDOW) || (!xf86VTSema)) {
        switch (pDrawable->depth) {
            case 1:
		mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
                break;
            case 8:
		cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
                break;
            default:
                ErrorF("Unsupported pixmap depth\n");
                break;
        }
        return;
    }

    pixmapStride = PixmapBytePad(wMax, pDrawable->depth);
    pdst = (unsigned char *)pdstStart;

    for (; nspans--; ppt++, pwidth++) {
	(mach32ImageReadFunc)(ppt->x, ppt->y, j = *pwidth, 1, pdst,
				pixmapStride, 0, 0, ~0);
	pdst += j;		/* width is in 32 bit words */
	j = (-j) & 3;
	while (j--)		/* Pad out to 32-bit boundary */
	    *pdst++ = 0;
    }

    WaitIdleEmpty(); /* Make sure that all commands have finished */
}
