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
/* $XConsortium: mfbimage.c,v 5.1 89/07/26 15:50:51 rws Exp $ */

#include "X.h"

#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"

#include "cfb.h"
#include "mi.h"
#include "Xmd.h"

#include "cfbmskbits.h"

#include "servermd.h"

/*ARGSUSED*/
void
cfbPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr dst;
    GCPtr	pGC;
    int		depth, x, y, w, h;
    int leftPad;
    unsigned int format;
    int 	*pImage;
{
    pointer	pbits;
    PixmapRec	FakePixmap;

    /* 0 may confuse CreatePixmap, and will sometimes be
       passed by the mi text code
    */
    if ((w == 0) || (h == 0))
	return;

    if (format == ZPixmap && depth == PSZ)
    {
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = dst->pScreen;
    	FakePixmap.drawable.depth = depth;
    	FakePixmap.drawable.bitsPerPixel = depth;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w+leftPad;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = PixmapBytePad(FakePixmap.drawable.width, depth);
    	FakePixmap.refcnt = 1;
    	FakePixmap.devPrivate.ptr = (pointer)pImage;
    	((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = FALSE;
	(*pGC->ops->CopyArea)(&FakePixmap, dst, pGC, leftPad, 0, w, h, x, y);
	((cfbPrivGC*)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = TRUE;
    }
    else if (format == XYBitmap && depth == 1)
    {
	/* taking a few liberties on the internals of mfb ...  */
	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
	FakePixmap.drawable.class = 0;
	FakePixmap.drawable.pScreen = dst->pScreen;
	FakePixmap.drawable.depth = 1;
	FakePixmap.drawable.bitsPerPixel = 1;
	FakePixmap.drawable.id = 0;
	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
	FakePixmap.drawable.x = 0;
	FakePixmap.drawable.y = 0;
	FakePixmap.drawable.width = w+leftPad;
	FakePixmap.drawable.height = h;
	FakePixmap.devKind = PixmapBytePad(FakePixmap.drawable.width, 1);
	FakePixmap.refcnt = 1;
	FakePixmap.devPrivate.ptr = (pointer)pImage;
    	((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = FALSE;
	(*pGC->ops->CopyPlane)(&FakePixmap, dst, pGC, leftPad, 0, w, h, x, y, 1);
	((cfbPrivGC*)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = TRUE;
    }
    else
    {
	miPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, pImage);
    }
}

/*
 * pdstLine points to space allocated by caller, which he can do since
 * he knows dimensions of the pixmap
 * we can call cfbDoBitblt because the dispatcher has promised not to send us
 * anything that would require going over the edge of the screen.
 *
 */
/*ARGSUSED*/
void
cfbGetImage( pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    int xorg, yorg;
    PixmapRec	FakePixmap;
    BoxRec box;
    DDXPointRec ptSrc;
    RegionPtr prgnDst;
    pointer pspare;
    GCPtr   pGC;

    if (format == ZPixmap)
    {
	pGC = GetScratchGC (PSZ, pDrawable->pScreen;
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDrawable->pScreen;
    	FakePixmap.drawable.depth = PSZ;
    	FakePixmap.drawable.bitsPerPixel = PSZ;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = PixmapBytePad(FakePixmap.drawable.width, PSZ);
    	FakePixmap.refcnt = 1;
    	FakePixmap.devPrivate.ptr = (pointer)pdstLine;
	if ((planeMask & PIM) != PIM)
    	{
	    unsigned long   zero = 0;
	    xRectangle	    fill;
	    DoChangeGC (pGC, GXforeground, &zero);
	    ValidateGC (&FakePixmap, pGC);
	    fill.x = 0;
	    fill.y = 0;
	    fill.width = w;
	    fill.height = h;
	    (*pGC->ops->FillRectangle) (&FakePixmap, pGC, 1, &fill);
	    DoChangeGC (pGC, GXplaneMask, (XID *) &planeMask);
	}
	ValidateGC (&FakePixmap, pGC);
	(*pGC->ops->CopyArea)(pDrawable, &FakePixmap, pGC, sx, sy, w, h, 0, 0);
	FreeScratchGC (pGC);
    }
    else
    {
	miGetImage (pDraw, sx, sy, w, h, format, planeMask, pdstLine);
    }
}
