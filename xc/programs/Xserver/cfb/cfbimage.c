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
/* $XConsortium: cfbimage.c,v 1.9 91/12/19 18:36:36 keith Exp $ */

#include "X.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"

extern void mfbGetImage();

void
cfbPutImage(pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		depth, x, y, w, h;
    int		leftPad;
    unsigned int format;
    char 	*pImage;
{
    int		bitsPerPixel;
    PixmapPtr   pPixmap;

    if ((w == 0) || (h == 0))
	return;

    if (format != XYPixmap)
    {
	pPixmap = GetScratchPixmapHeader(pDraw->pScreen, w+leftPad, h, depth,
		      BitsPerPixel(depth), PixmapBytePad(w+leftPad, depth),
		      (pointer)pImage);
	if (!pPixmap)
	    return;
	
    	((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = FALSE;
	if (format == ZPixmap)
	    (void)(*pGC->ops->CopyArea)(pPixmap, pDraw, pGC, leftPad, 0,
					w, h, x, y);
	else
	    (void)(*pGC->ops->CopyPlane)(pPixmap, pDraw, pGC, leftPad, 0,
					 w, h, x, y, 1);
	((cfbPrivGC*)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = TRUE;
        FreeScratchPixmapHeader(pPixmap);
    }
    else
    {
	unsigned long	oldFg, oldBg, gcv[3];
	unsigned long	oldPlanemask;
	unsigned long	i;
	long		bytesPer;

	depth = pGC->depth;
	oldPlanemask = pGC->planemask;
	oldFg = pGC->fgPixel;
	oldBg = pGC->bgPixel;
	gcv[0] = ~0L;
	gcv[1] = 0;
	DoChangeGC(pGC, GCForeground | GCBackground, gcv, 0);
	bytesPer = (long)h * BitmapBytePad(w + leftPad);

	for (i = 1 << (depth-1); i != 0; i >>= 1, pImage += bytesPer)
	{
	    if (i & oldPlanemask)
	    {
	        gcv[0] = i;
	        DoChangeGC(pGC, GCPlaneMask, gcv, 0);
	        ValidateGC(pDraw, pGC);
	        (*pGC->ops->PutImage)(pDraw, pGC, 1, x, y, w, h, leftPad,
			         XYBitmap, pImage);
	    }
	}
	gcv[0] = oldPlanemask;
	gcv[1] = oldFg;
	gcv[2] = oldBg;
	DoChangeGC(pGC, GCPlaneMask | GCForeground | GCBackground, gcv, 0);
    }
}

void
cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    BoxRec box;
    DDXPointRec ptSrc;
    RegionRec rgnDst;
    ScreenPtr pScreen;
    PixmapPtr pPixmap;

    if ((w == 0) || (h == 0))
	return;
    if (pDrawable->bitsPerPixel == 1)
    {
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }
    pScreen = pDrawable->pScreen;
    if (format == ZPixmap)
    {
	pPixmap = GetScratchPixmapHeader(pScreen, w, h, 
			pDrawable->depth, pDrawable->bitsPerPixel,
			PixmapBytePad(w,pDrawable->depth), (pointer)pdstLine);
	if (!pPixmap)
	    return;
	if ((planeMask & PMSK) != PMSK)
	    bzero((char *)pdstLine, pPixmap->devKind * h);
        ptSrc.x = sx + pDrawable->x;
        ptSrc.y = sy + pDrawable->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pScreen->RegionInit)(&rgnDst, &box, 1);
	cfbDoBitblt(pDrawable, (DrawablePtr)pPixmap, GXcopy, &rgnDst,
		    &ptSrc, planeMask);
        (*pScreen->RegionUninit)(&rgnDst);
	FreeScratchPixmapHeader(pPixmap);
    }
    else
    {
#if PSZ == 8
	pPixmap = GetScratchPixmapHeader(pScreen, w, h,  /*depth*/ 1,
			/*bpp*/ 1, BitmapBytePad(w), (pointer)pdstLine);
	if (!pPixmap)
	    return;

        ptSrc.x = sx + pDrawable->x;
        ptSrc.y = sy + pDrawable->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pScreen->RegionInit)(&rgnDst, &box, 1);
	cfbCopyImagePlane (pDrawable, (DrawablePtr)pPixmap, GXcopy, &rgnDst,
		    &ptSrc, planeMask);
        (*pScreen->RegionUninit)(&rgnDst);
	FreeScratchPixmapHeader(pPixmap);
#else
	miGetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
#endif
    }
}
