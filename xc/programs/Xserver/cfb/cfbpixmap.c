/* $XConsortium: cfbpixmap.c,v 5.9 93/06/24 10:26:04 dpw Exp $ */
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
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "mi.h"
#include "cfb.h"
#include "cfbmskbits.h"

extern unsigned int endtab[];

PixmapPtr
cfbCreatePixmap (pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    PixmapPtr pPixmap;
    int datasize;
    int paddedWidth;

    paddedWidth = PixmapBytePad(width, depth);
    datasize = height * paddedWidth;
    pPixmap = AllocatePixmap(pScreen, datasize);
    if (!pPixmap)
	return NullPixmap;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = BitsPerPixel(depth);
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = paddedWidth;
    pPixmap->refcnt = 1;
#ifdef PIXPRIV
    pPixmap->devPrivate.ptr = datasize ?
		(pointer)((char *)pPixmap + pScreen->totalPixmapSize) : NULL;
#else
    pPixmap->devPrivate.ptr = (pointer)(pPixmap + 1);
#endif
    return pPixmap;
}

Bool
cfbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    if(--pPixmap->refcnt)
	return TRUE;
    xfree(pPixmap);
    return TRUE;
}

PixmapPtr
cfbCopyPixmap(pSrc)
    register PixmapPtr	pSrc;
{
    register PixmapPtr	pDst;
    int		size;
    ScreenPtr pScreen;

    size = pSrc->drawable.height * pSrc->devKind;
    pScreen = pSrc->drawable.pScreen;
    pDst = (*pScreen->CreatePixmap) (pScreen, pSrc->drawable.width, 
				pSrc->drawable.height, pSrc->drawable.depth);
    if (!pDst)
	return NullPixmap;
    bcopy((char *)pSrc->devPrivate.ptr, (char *)pDst->devPrivate.ptr, size);
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
*/
void
cfbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    register int width = (pPixmap->drawable.width) * (pPixmap->drawable.bitsPerPixel);
    register int h;
    register int mask;
    register unsigned int *p;
    register unsigned int bits; /* real pattern bits */
    register int i;
    int rep;                    /* repeat count for pattern */
 
    if (width >= 32)
        return;

    rep = 32/width;
    if (rep*width != 32)
        return;
 
    mask = endtab[width];
 
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
    pPixmap->drawable.width = 32/(pPixmap->drawable.bitsPerPixel);
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
#endif /* notdef */

/* Rotates pixmap pPix by w pixels to the right on the screen. Assumes that
 * words are 32 bits wide, and that the least significant bit appears on the
 * left.
 */
void
cfbXRotatePixmap(pPix, rw)
    PixmapPtr	pPix;
    register int rw;
{
    register unsigned int	*pw, *pwFinal;
    register unsigned int	t;
    int				rot;

    if (pPix == NullPixmap)
        return;

    switch (((DrawablePtr) pPix)->bitsPerPixel) {
	case PSZ:
	    break;
	case 1:
	    mfbXRotatePixmap(pPix, rw);
	    return;
	default:
	    ErrorF("cfbXRotatePixmap: unsupported bitsPerPixel %d\n", ((DrawablePtr) pPix)->bitsPerPixel);
	    return;
    }
    pw = (unsigned int *)pPix->devPrivate.ptr;
    modulus (rw, (int) pPix->drawable.width, rot);
    if(pPix->drawable.width == PPW)
    {
        pwFinal = pw + pPix->drawable.height;
	while(pw < pwFinal)
	{
	    t = *pw;
	    *pw++ = SCRRIGHT(t, rot) |
		    (SCRLEFT(t, (PPW-rot)) & cfbendtab[rot]);
	}
    }
    else
    {
        ErrorF("cfb internal error: trying to rotate odd-sized pixmap.\n");
#ifdef notdef
	register unsigned int *pwTmp;
	int size, tsize;

	tsize = PixmapBytePad(pPix->drawable.width - rot, pPix->drawable.depth);
	pwTmp = (unsigned int *) ALLOCATE_LOCAL(pPix->drawable.height * tsize);
	if (!pwTmp)
	    return;
	/* divide pw (the pixmap) in two vertically at (w - rot) and swap */
	tsize >>= 2;
	size = pPix->devKind >> 2;
	cfbQuickBlt((int *)pw, (int *)pwTmp,
		    0, 0, 0, 0,
		    (int)pPix->drawable.width - rot, (int)pPix->drawable.height,
		    size, tsize);
	cfbQuickBlt((int *)pw, (int *)pw,
		    (int)pPix->drawable.width - rot, 0, 0, 0,
		    rot, (int)pPix->drawable.height,
		    size, size);
	cfbQuickBlt((int *)pwTmp, (int *)pw,
		    0, 0, rot, 0,
		    (int)pPix->drawable.width - rot, (int)pPix->drawable.height,
		    tsize, size);
	DEALLOCATE_LOCAL(pwTmp);
#endif
    }
}

/* Rotates pixmap pPix by h lines.  Assumes that h is always less than
   pPix->drawable.height
   works on any width.
 */
void
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
    int	rot;

    if (pPix == NullPixmap)
	return;
    switch (((DrawablePtr) pPix)->bitsPerPixel) {
	case PSZ:
	    break;
	case 1:
	    mfbYRotatePixmap(pPix, rh);
	    return;
	default:
	    ErrorF("cfbYRotatePixmap: unsupported bitsPerPixel %d\n", ((DrawablePtr) pPix)->bitsPerPixel);
	    return;
    }

    modulus (rh, (int) pPix->drawable.height, rot);
    pbase = (char *)pPix->devPrivate.ptr;

    nbyDown = rot * pPix->devKind;
    nbyUp = (pPix->devKind * pPix->drawable.height) - nbyDown;
    if(!(ptmp = (char *)ALLOCATE_LOCAL(nbyUp)))
	return;

    bcopy(pbase, ptmp, nbyUp);		/* save the low rows */
    bcopy(pbase+nbyUp, pbase, nbyDown);	/* slide the top rows down */
    bcopy(ptmp, pbase+nbyDown, nbyUp);	/* move lower rows up to row rot */
    DEALLOCATE_LOCAL(ptmp);
}

void
cfbCopyRotatePixmap(psrcPix, ppdstPix, xrot, yrot)
    register PixmapPtr psrcPix, *ppdstPix;
    int	xrot, yrot;
{
    register PixmapPtr pdstPix;

    if ((pdstPix = *ppdstPix) &&
	(pdstPix->devKind == psrcPix->devKind) &&
	(pdstPix->drawable.height == psrcPix->drawable.height))
    {
	bcopy((char *)psrcPix->devPrivate.ptr,
	      (char *)pdstPix->devPrivate.ptr,
	      psrcPix->drawable.height * psrcPix->devKind);
	pdstPix->drawable.width = psrcPix->drawable.width;
	pdstPix->drawable.depth = psrcPix->drawable.depth;
	pdstPix->drawable.bitsPerPixel = psrcPix->drawable.bitsPerPixel;
	pdstPix->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    }
    else
    {
	if (pdstPix)
	    cfbDestroyPixmap(pdstPix);
	*ppdstPix = pdstPix = cfbCopyPixmap(psrcPix);
	if (!pdstPix)
	    return;
    }
    cfbPadPixmap(pdstPix);
    if (xrot)
	cfbXRotatePixmap(pdstPix, xrot);
    if (yrot)
	cfbYRotatePixmap(pdstPix, yrot);
}
