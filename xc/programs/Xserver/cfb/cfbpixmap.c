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
/* pixmap management
   written by drewry, september 1986

   on a monchrome device, a pixmap is a bitmap.
*/

#include "Xmd.h"
#include "servermd.h"
#include "pixmapstr.h"
#include "cfbmskbits.h"

#include "cfb.h"
#include "mi.h"

extern void cfbXRotateBitmap();

#if (BITMAP_BIT_ORDER == MSBFirst)
static int masktab[32] = 
    {
        0x00000000,
        0x80000000,
        0xC0000000,
        0xE0000000,
        0xF0000000,
        0xF8000000,
        0xFC000000,
        0xFE000000,
        0xFF000000,
        0xFF800000,
        0xFFC00000,
        0xFFE00000,
        0xFFF00000,
        0xFFF80000,
        0xFFFC0000,
        0xFFFE0000,
        0xFFFF0000,
        0xFFFF8000,
        0xFFFFC000,
        0xFFFFE000,
        0xFFFFF000,
        0xFFFFF800,
        0xFFFFFC00,
        0xFFFFFE00,
        0xFFFFFF00,
        0xFFFFFF80,
        0xFFFFFFC0,
        0xFFFFFFE0,
        0xFFFFFFF0,
        0xFFFFFFF8,
        0xFFFFFFFC,
        0xFFFFFFFE
    };
#else
static int masktab[32] =
        {
        0x00000000,
        0x00000001,
        0x00000003,
        0x00000007,
        0x0000000F,
        0x0000001F,
        0x0000003F,
        0x0000007F,
        0x000000FF,
        0x000001FF,
        0x000003FF,
        0x000007FF,
        0x00000FFF,
        0x00001FFF,
        0x00003FFF,
        0x00007FFF,
        0x0000FFFF,
        0x0001FFFF,
        0x0003FFFF,
        0x0007FFFF,
        0x000FFFFF,
        0x001FFFFF,
        0x003FFFFF,
        0x007FFFFF,
        0x00FFFFFF,
        0x01FFFFFF,
        0x03FFFFFF,
        0x07FFFFFF,
        0x0FFFFFFF,
        0x1FFFFFFF,
        0x3FFFFFFF,
        0x7FFFFFFF
        };
#endif

PixmapPtr
cfbCreatePixmap (pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    int size;

    if (depth != 1 && depth != PSZ)
	return NullPixmap;

    pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec));
    if (!pPixmap)
	return NullPixmap;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = PixmapBytePad(width, depth);
    pPixmap->refcnt = 1;
    size = height * pPixmap->devKind;

    if ( !(pPixmap->devPrivate.ptr = (pointer)xalloc(size)))
    {
	xfree(pPixmap);
	return NullPixmap;
    }
    return pPixmap;
}

Bool
cfbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    if(--pPixmap->refcnt)
	return TRUE;
    xfree(pPixmap->devPrivate.ptr);
    xfree(pPixmap);
    return TRUE;
}


PixmapPtr
cfbCopyPixmap(pSrc)
    register PixmapPtr	pSrc;
{
    register PixmapPtr	pDst;
    register int	*pDstPriv, *pSrcPriv, *pDstMax;
    int		size;

    pDst = (PixmapPtr) xalloc(sizeof(PixmapRec));
    if (!pDst)
	return NullPixmap;
    pDst->drawable.type = pSrc->drawable.type;
    pDst->drawable.pScreen = pSrc->drawable.pScreen;
    pDst->drawable.x = pSrc->drawable.x;
    pDst->drawable.y = pSrc->drawable.y;
    pDst->drawable.width = pSrc->drawable.width;
    pDst->drawable.height = pSrc->drawable.height;
    pDst->drawable.depth = pSrc->drawable.depth;
    pDst->devKind = pSrc->devKind;
    pDst->refcnt = 1;

    size = pDst->drawable.height * pDst->devKind;
    pDstPriv = (int *) xalloc(size);
    if (!pDstPriv)
    {
	xfree(pDst);
	return NullPixmap;
    }
    bzero((char *) pDstPriv, size);
    pDst->devPrivate.ptr = (pointer) pDstPriv;
    pSrcPriv = (int *)pSrc->devPrivate.ptr;
    pDstMax = pDstPriv + (size >> 2);
    /* Copy words */
    while(pDstPriv < pDstMax)
    {
        *pDstPriv++ = *pSrcPriv++;
    }

    return pDst;
}


/* replicates a pattern to be a full 32 bits wide.
   relies on the fact that each scnaline is longword padded.
   doesn't do anything if pixmap is not a factor of 32 wide.
   changes width field of pixmap if successful, so that the fast
	cfbXRotatePixmap code gets used if we rotate the pixmap later.
	cfbYRotatePixmap code gets used if we rotate the pixmap later.

   calculate number of times to repeat
   for each scanline of pattern
      zero out area to be filled with replicate
      left shift and or in original as many times as needed

   returns TRUE iff pixmap was, or could be padded to be, 32 bits wide.
*/
Bool
cfbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    register int width = (pPixmap->drawable.width) * (pPixmap->drawable.depth);
    register int h;
    register int mask;
    register unsigned int *p;
    register unsigned int bits; /* real pattern bits */
    register int i;
    int rep;                    /* repeat count for pattern */
 
    if (width == 32)
        return(TRUE);
    if (width > 32)
        return(FALSE);
 
    rep = 32/width;
    if (rep*width != 32)
        return(FALSE);
 
    mask = masktab[width];
 
    p = (unsigned int *)(pPixmap->devPrivate.ptr);
    for (h=0; h < pPixmap->drawable.height; h++)
    {
        *p &= mask;
        bits = *p;
        for(i=1; i<rep; i++)
        {
#if (BITMAP_BIT_ORDER == MSBFirst) 
            bits >>= width;
#else
	    bits <<= width;
#endif
            *p |= bits;
        }
        p++;
    }    
    pPixmap->drawable.width = 32/(pPixmap->drawable.depth);
    return(TRUE);
}


#ifdef notdef
/*
 * cfb debugging routine -- assumes pixmap is 1 byte deep 
 */
static cfbdumppixmap(pPix)
    PixmapPtr	pPix;
{
    unsigned int *pw;
    char *psrc, *pdst;
    int	i, j;
    char	line[66];

    ErrorF(  "pPixmap: 0x%x\n", pPix);
    ErrorF(  "%d wide %d high\n", pPix->drawable.width, pPix->drawable.height);
    if (pPix->drawable.width > 64)
    {
	ErrorF(  "too wide to see\n");
	return;
    }

    pw = (unsigned int *) pPix->devPrivate.ptr;
    psrc = (char *) pw;

/*
    for ( i=0; i<pPix->drawable.height; ++i )
	ErrorF( "0x%x\n", pw[i] );
*/

    for ( i = 0; i < pPix->drawable.height; ++i ) {
	pdst = line;
	for(j = 0; j < pPix->drawable.width; j++) {
	    *pdst++ = *psrc++ ? 'X' : ' ' ;
	}
	*pdst++ = '\n';
	*pdst++ = '\0';
	ErrorF( "%s", line);
    }
}
#endif notdef

/* Rotates pixmap pPix by w pixels to the right on the screen. Assumes that
 * words are 32 bits wide, and that the least significant bit appears on the
 * left.
 */
cfbXRotatePixmap(pPix, rw)
    PixmapPtr	pPix;
    register int rw;
{
    register unsigned int	*pw, *pwFinal, *pwNew;
    register unsigned int	t;
    int			size;

    if (pPix == NullPixmap)
        return;

    switch (((DrawablePtr) pPix)->depth) {
	case PSZ:
	    break;
	case 1:
	    cfbXRotateBitmap(pPix, rw);
	    return;
	default:
	    ErrorF("cfbXRotatePixmap: unsupported depth %d\n", ((DrawablePtr) pPix)->depth);
	    return;
    }
    pw = (unsigned int *)pPix->devPrivate.ptr;
    rw %= pPix->drawable.width;
    if (rw < 0)
	rw += pPix->drawable.width;
    if(pPix->drawable.width == PPW)
    {
        pwFinal = pw + pPix->drawable.height;
	while(pw < pwFinal)
	{
	    t = *pw;
	    *pw++ = SCRRIGHT(t, rw) |
		    (SCRLEFT(t, (PPW-rw)) & cfbendtab[rw]);
	}
    }
    else
    {
	pwNew = (unsigned int *) xalloc( pPix->drawable.height * pPix->devKind);
	if (!pwNew)
	    return;

	/* o.k., divide pw (the pixmap) in two vertically at (w - rw)
	 * pick up the part on the left and make it the right of the new
	 * pixmap.  then pick up the part on the right and make it the left
	 * of the new pixmap.
	 * now hook in the new part and throw away the old. All done.
	 */
	size = PixmapBytePad(pPix->drawable.width, PSZ) >> 2;
        cfbQuickBlt((int *)pw, (int *)pwNew, 0, 0, rw, 0,
		    pPix->drawable.width - rw, pPix->drawable.height, size, size);
        cfbQuickBlt((int *)pw, (int *)pwNew, pPix->drawable.width - rw, 0, 0, 0,
		    rw, pPix->drawable.height, size, size);
	pPix->devPrivate.ptr = (pointer) pwNew;
	xfree(pw);

    }

}
/* Rotates pixmap pPix by h lines.  Assumes that h is always less than
   pPix->drawable.height
   works on any width.
 */
cfbYRotatePixmap(pPix, rh)
    register PixmapPtr	pPix;
    int	rh;
{
    int nbyDown;	/* bytes to move down to row 0; also offset of
			   row rh */
    int nbyUp;		/* bytes to move up to line rh; also
			   offset of first line moved down to 0 */
    char *pbase;
    char *ptmp;

    if (pPix == NullPixmap)
	return;
    switch (((DrawablePtr) pPix)->depth) {
	case PSZ:
	    break;
	case 1:
	    mfbYRotatePixmap(pPix, rh);
	    return;
	default:
	    ErrorF("cfbYRotatePixmap: unsupported depth %d\n", ((DrawablePtr) pPix)->depth);
	    return;
    }

    rh %= pPix->drawable.height;
    if (rh < 0)
	rh += pPix->drawable.height;

    pbase = (char *)pPix->devPrivate.ptr;

    nbyDown = rh * pPix->devKind;
    nbyUp = (pPix->devKind * pPix->drawable.height) - nbyDown;
    if(!(ptmp = (char *)ALLOCATE_LOCAL(nbyUp)))
	return;

    bcopy(pbase, ptmp, nbyUp);		/* save the low rows */
    bcopy(pbase+nbyUp, pbase, nbyDown);	/* slide the top rows down */
    bcopy(ptmp, pbase+nbyDown, nbyUp);	/* move lower rows up to row rh */
    DEALLOCATE_LOCAL(ptmp);
}

