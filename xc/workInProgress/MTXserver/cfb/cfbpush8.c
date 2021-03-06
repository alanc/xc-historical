/*
 * Push Pixels for 8 bit displays.
 */

/*

Copyright (c) 1989  X Consortium

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

Copyright 1992, 1993 Data General Corporation;
Copyright 1992, 1993 OMRON Corporation  

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
neither the name OMRON or DATA GENERAL be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission of the party whose name is to be used.  Neither OMRON or 
DATA GENERAL make any representation about the suitability of this software
for any purpose.  It is provided "as is" without express or implied warranty.  

OMRON AND DATA GENERAL EACH DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OMRON OR DATA GENERAL BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

*/
/* $XConsortium: cfbpush8.c,v 1.5 94/04/17 21:16:25 rob Exp $ */

#if PSZ == 8

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#define MFB_CONSTS_ONLY
#include	"maskbits.h"

void
cfbPushPixels8 (pGC, pBitmap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr	pGC;
    PixmapPtr	pBitmap;
    DrawablePtr	pDrawable;
    int		dx, dy, xOrg, yOrg;
{
    register unsigned long   *src, *dst;
    register unsigned long   pixel;
    register unsigned long   c, bits;
    unsigned long   *pdstLine, *psrcLine;
    unsigned long   *pdstBase;
    int		    srcWidth;
    int		    dstWidth;
    int		    xoff;
    int		    nBitmapLongs, nPixmapLongs;
    int		    nBitmapTmp, nPixmapTmp;
    unsigned long   rightMask;
    BoxRec	    bbox;
    cfbPrivGCPtr    devPriv;
#if defined(XTHREADS) && defined(TRANSLATE_COORDS)
    int xorg, yorg;

    xorg = xOrg;
    yorg = yOrg;
    xOrg += pDrawable->x;
    yOrg += pDrawable->y;
#endif /* XTHREADS */

    bbox.x1 = xOrg;
    bbox.y1 = yOrg;
    bbox.x2 = bbox.x1 + dx;
    bbox.y2 = bbox.y1 + dy;
    devPriv = cfbGetGCPrivate(pGC);
    
    switch (RECT_IN_REGION(pGC->pScreen, devPriv->pCompositeClip, &bbox))
    {
      case rgnPART:
#if defined(XTHREADS) && defined(TRANSLATE_COORDS)
	mfbPushPixels(pGC, pBitmap, pDrawable, dx, dy, xorg, yorg);
#else /* XTHREADS */
	mfbPushPixels(pGC, pBitmap, pDrawable, dx, dy, xOrg, yOrg);
#endif /* XTHREADS */
      case rgnOUT:
	return;
    }

    cfbGetLongWidthAndPointer (pDrawable, dstWidth, pdstBase)

    psrcLine = (unsigned long *) pBitmap->devPrivate.ptr;
    srcWidth = (int) pBitmap->devKind >> PWSH;
    
    pixel = devPriv->xor;
    xoff = xOrg & PIM;
    nBitmapLongs = (dx + xoff) >> MFB_PWSH;
    nPixmapLongs = (dx + PGSZB + xoff) >> PWSH;

    rightMask = ~cfb8BitLenMasks[((dx + xoff) & MFB_PIM)];

    pdstLine = pdstBase + (yOrg * dstWidth) + (xOrg >> PWSH);

    while (dy--)
    {
	c = 0;
	nPixmapTmp = nPixmapLongs;
	nBitmapTmp = nBitmapLongs;
	src = psrcLine;
	dst = pdstLine;
	while (nBitmapTmp--)
	{
	    bits = *src++;
	    c |= BitRight (bits, xoff);
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	    nPixmapTmp -= 8;
	    c = 0;
	    if (xoff)
		c = BitLeft (bits, PGSZ - xoff);
	}
	if (BitLeft (rightMask, xoff))
	    c |= BitRight (*src, xoff);
	c &= rightMask;
	switch (nPixmapTmp) {
	case 8:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 7:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 6:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 5:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 4:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 3:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 2:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 1:
	    WriteBitGroup(dst, pixel, GetBitGroup(c));
	    NextBitGroup(c);
	    dst++;
	case 0:
	    break;
	}
	pdstLine += dstWidth;
	psrcLine += srcWidth;
    }
}

#endif
