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

/* $XConsortium: cfbteblt8.c,v 5.1 89/07/28 12:51:24 keith Exp $ */

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

#include "cfb8bit.h"

extern void miImageGlyphBlt();

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

    if (!cfb8CheckPixels (pGC->fgPixel, pGC->bgPixel))
	cfb8SetPixels (pGC->fgPixel, pGC->bgPixel);

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
			c = BitRight (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (GetFourPixels(c) & rightMask);
			dst += widthDst;
		    }
		    break;
		case 1:
		    while (hTmp--) {
			c = BitRight (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (GetFourPixels (c) & rightMask);
			NextFourBits(c);
			dst[1] = GetFourPixels(c);
			dst += widthDst;
		    }
		    break;
		case 2:
		    while (hTmp--) {
			c = BitRight (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (GetFourPixels (c) & rightMask);
			NextFourBits(c);
			dst[1] = GetFourPixels(c);
			NextFourBits(c);
			dst[2] = GetFourPixels(c);
			dst += widthDst;
		    }
		    break;
		case 3:
		    while (hTmp--) {
			c = BitRight (*rightChar++, xoff);
			*dst = (*dst & leftMask) |
			       (GetFourPixels (c) & rightMask);
			NextFourBits(c);
			dst[1] = GetFourPixels(c);
			NextFourBits(c);
			dst[2] = GetFourPixels(c);
			NextFourBits(c);
			dst[3] = GetFourPixels(c);
			dst += widthDst;
		    }
		    break;
		default:
		    widthDiff = widthDst - ew;
		    dst -= widthDiff;
		    while (hTmp--) {
			c = BitRight (*rightChar++, xoff);
			dst += widthDiff;
			*dst = (*dst & leftMask) |
			       (GetFourPixels (c) & rightMask);
			ewTmp = ew;
			while (ewTmp--) {
			    dst++;
			    NextFourBits(c);
			    *dst = GetFourPixels (c);
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
			    BitRight (*rightChar++, xoff);
			dst[0] = GetFourPixels(c);
		    	dst += widthDst;
		    }
		    break;
		case 2:
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    BitRight (*rightChar++, xoff);
			dst[0] = GetFourPixels(c);
			NextFourBits(c);
			dst[1] = GetFourPixels(c);
		    	dst += widthDst;
		    }
		    break;
		case 3:
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    BitRight (*rightChar++, xoff);
			dst[0] = GetFourPixels(c);
			NextFourBits(c);
			dst[1] = GetFourPixels(c);
			NextFourBits(c);
			dst[2] = GetFourPixels(c);
		    	dst += widthDst;
		    }
		    break;
		default:
		    widthDiff = widthDst - ew;
		    dst = dst - widthDiff;
		    while (hTmp--) {
		    	c = BitLeft (*leftChar++, lshift) |
			    BitRight(*rightChar++,xoff);
		    	dst += widthDiff;
		    	ewTmp = ew;
		    	while (ewTmp--) {
			    *dst = GetFourPixels(c);
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
		    dst[0] = GetFourPixels(c);
		    dst += widthDst;
	    	}
		break;
	    case 2:
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst[0] = GetFourPixels(c);
		    NextFourBits(c);
		    dst[1] = GetFourPixels(c);
		    dst += widthDst;
	    	}
		break;
	    case 3:
	    	while (hTmp--) {
		    c = *rightChar++;
		    dst[0] = GetFourPixels(c);
		    NextFourBits(c);
		    dst[1] = GetFourPixels(c);
		    NextFourBits(c);
		    dst[2] = GetFourPixels(c);
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
		    	*dst++ = GetFourPixels(c);
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
		   (GetFourPixels(BitLeft (*leftChar, lshift)) & leftMask);
	    leftChar++;
	    dst += widthDst;
	}
    }
}
#endif /* PPW == 4 */
