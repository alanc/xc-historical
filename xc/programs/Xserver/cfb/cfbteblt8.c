/*
 * TEGblt - ImageText expanded glyph fonts only.  For
 * 8 bit displays, in Copy mode with no clipping.
 */

/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
*/

/* $XConsortium: Exp $ */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"

#if (PPW == 4)
static unsigned long	pixels[16];
static unsigned long	fgPixel, bgPixel;

#if (BITMAP_BIT_ORDER == MSBFirst)
#define GetFourBits(x)		(((x) >> 28) & 0xf)
#define ToXoff(x,xoff)		((x) >> (xoff))
#define NextFourBits(x)		((x) <<= 4)
#define BitLeft(x,y)		((x) << (y))
#else
#define GetFourBits(x)		((x) & 0xf)
#define ToXoff(x,xoff)		((x) << (xoff))
#define NextFourBits(x)		((x) >>= 4)
#define BitLeft(x,y)		((x) >> (y))
#endif

void
cfbTEGlyphBlt8 (pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	xInit, yInit;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    register unsigned long  c;
    register unsigned long  *dst;
    register int	    ewTmp;
    register int	    xoff;
    register unsigned long  *rightChar;
    register unsigned long  leftMask, rightMask;

    CharInfoPtr		pci;
    FontInfoPtr		pfi = pGC->font->pFI;
    unsigned long	*dstLine;
    unsigned long	*oldRightChar;
    unsigned long	*pdstBase;
    unsigned long	*leftChar;
    int			s;
    int			hTmp;
    int			widthDst;
    int			widthGlyph;
    int			h;
    int			ew;
    int			x, y;
    BoxRec		bbox;		/* for clipping */
    int			lshift;
    int			widthDiff;

    pci = &pfi->maxbounds;
    widthGlyph = pci->metrics.characterWidth;
    h = pfi->fontAscent + pfi->fontDescent;
    if (!h)
	return;
    x = xInit + pci->metrics.leftSideBearing + pDrawable->x;
    y = yInit - pfi->fontAscent + pDrawable->y;
    bbox.x1 = x;
    bbox.x2 = x + (widthGlyph * nglyph);
    bbox.y1 = y;
    bbox.y2 = y + h;

    switch ((*pGC->pScreen->RectIn)(
                ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip, &bbox))
    {
      case rgnPART:
	miImageGlyphBlt(pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase);
      case rgnOUT:
	return;
    }

    if ((pGC->fgPixel & 0xff) != fgPixel || (pGC->bgPixel & 0xff) != bgPixel)
    {
	fgPixel = pGC->fgPixel & 0xff;
	bgPixel = pGC->bgPixel & 0xff;
    	/*
     	 * create the appropriate pixel-fill bits for current
     	 * foreground
     	 */
    	for (s = 0; s < 16; s++)
    	{
	    c = 0;
	    if (s & 1)
	    	c |= 0xff;
	    if (s & 2)
	    	c |= 0xff00;
	     if (s & 4)
	    	c |= 0xff0000;
	     if (s & 8)
	    	c |= 0xff000000;
	    pixels[s] = (c & PFILL(fgPixel)) |
		    	(~c & PFILL(bgPixel));
    	}
    }

    leftChar = 0;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }
    pdstBase += y * widthDst;
    while (nglyph--)
    {
	xoff = x & 0x3;
	oldRightChar = rightChar = (unsigned long *) (pglyphBase + (*ppci++)->byteOffset);
	hTmp = h;
	dstLine = pdstBase + (x >> 2);
	if (xoff)
	{
	    if (!leftChar)
	    {
		leftMask = cfbendtab[xoff];
		rightMask = cfbstarttab[xoff];
		dst = dstLine;
		ew = (widthGlyph - (4 - xoff)) >> 2;
		switch (ew)
 		{
		case 0:
		    while (hTmp--) {
			c = ToXoff (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (pixels[GetFourBits (c)] & rightMask);
			dst += widthDst;
		    }
		    break;
		case 1:
		    while (hTmp--) {
			c = ToXoff (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (pixels[GetFourBits (c)] & rightMask);
			NextFourBits(c);
			dst[1] = pixels[GetFourBits(c)];
			dst += widthDst;
		    }
		    break;
		case 2:
		    while (hTmp--) {
			c = ToXoff (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (pixels[GetFourBits (c)] & rightMask);
			NextFourBits(c);
			dst[1] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[2] = pixels[GetFourBits(c)];
			dst += widthDst;
		    }
		    break;
		case 3:
		    while (hTmp--) {
			c = ToXoff (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (pixels[GetFourBits (c)] & rightMask);
			NextFourBits(c);
			dst[1] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[2] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[3] = pixels[GetFourBits(c)];
			dst += widthDst;
		    }
		    break;
		default:
		    widthDiff = widthDst - ew;
		    dst -= widthDiff;
		    while (hTmp--) {
			c = ToXoff (*rightChar++, xoff);
			dst += widthDiff;
			*dst = (*dst & leftMask) |
			       (pixels[GetFourBits (c)] & rightMask);
			ewTmp = ew;
			while (ewTmp--) {
			    dst++;
			    NextFourBits(c);
			    *dst = pixels[GetFourBits(c)];
			}
		    }
		    break;
		}
	    }
	    else
	    {
		ew = ((widthGlyph - (4 - xoff)) >> 2) + 1;
		lshift = widthGlyph - xoff;
		dst = dstLine;
		switch (ew) {
		case 1:
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    ToXoff (*rightChar++, xoff);
			dst[0] = pixels[GetFourBits(c)];
		    	dst += widthDst;
		    }
		    break;
		case 2:
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    ToXoff (*rightChar++, xoff);
			dst[0] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[1] = pixels[GetFourBits(c)];
		    	dst += widthDst;
		    }
		    break;
		case 3:
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    ToXoff (*rightChar++, xoff);
			dst[0] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[1] = pixels[GetFourBits(c)];
			NextFourBits(c);
			dst[2] = pixels[GetFourBits(c)];
		    	dst += widthDst;
		    }
		    break;
		default:
		    widthDiff = widthDst - ew;
		    dst = dst - widthDiff;
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    ToXoff(*rightChar++,xoff);
		    	dst += widthDiff;
		    	ewTmp = ew;
		    	while (ewTmp--) {
			    *dst = pixels[GetFourBits(c)];
			    ++dst;
			    NextFourBits(c);
		    	}
		    }
		}
	    }
	}
	else
	{
	    ew = widthGlyph >> 2;
	    dst = dstLine;
	    switch (ew) {
	    case 0:
		break;
	    case 1:
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst[0] = pixels[GetFourBits(c)];
		    dst += widthDst;
	    	}
		break;
	    case 2:
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst[0] = pixels[GetFourBits(c)];
		    NextFourBits(c);
		    dst[1] = pixels[GetFourBits(c)];
		    dst += widthDst;
	    	}
		break;
	    case 3:
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst[0] = pixels[GetFourBits(c)];
		    NextFourBits(c);
		    dst[1] = pixels[GetFourBits(c)];
		    NextFourBits(c);
		    dst[2] = pixels[GetFourBits(c)];
		    dst += widthDst;
	    	}
		break;
	    default:
		widthDiff = widthDst - ew;
		dst = dst - widthDiff;
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst += widthDiff;
		    ewTmp = ew;
		    while (ewTmp--) {
		    	*dst++ = pixels[GetFourBits(c)];
		    	NextFourBits(c);
		    }
	    	}
	    }
	}
	leftChar = oldRightChar;
	x += widthGlyph;
    }
    /*
     * draw the tail of the last character
     */
    xoff = x & 3;
    if (xoff)
    {
	rightMask = cfbstarttab[xoff];
	leftMask = cfbendtab[xoff];
	lshift = widthGlyph - xoff;
	dst = pdstBase + (x >> 2);
	hTmp = h;
	while (hTmp--)
	{
	    *dst = (*dst & rightMask) |
#if (BITMAP_BIT_ORDER == MSBFirst)
		   (pixels[GetFourBits(*leftChar << lshift)] & leftMask);
#else
		   (pixels[GetFourBits(*leftChar >> lshift)] & leftMask);
#endif
	    leftChar++;
	    dst += widthDst;
	}
    }
}
#endif /* PPW == 4 */
