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

/* $XConsortium: cfbteblt8.c,v 5.4 89/10/29 12:23:11 keith Exp $ */

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
#include	"cfb8bit.h"

#if (PPW == 4)

/* another ugly giant macro */
#define SwitchEm    switch (ew) \
		    { \
		    case 0: \
		    	break; \
		    case 1: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 \
			    Loop \
		    	} \
		    	break; \
		    case 2: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) \
			    Loop \
		    	} \
		    	break; \
		    case 3: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step StoreBits(2) \
			    Loop \
		    	} \
		    	break; \
		    case 4: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) \
			    Loop \
		    	} \
		    	break; \
		    case 5: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) \
			    Loop \
		    	} \
		    	break; \
		    case 6: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) \
			    Loop \
		    	} \
		    	break; \
		    case 7: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) Step \
			    StoreBits(6) \
			    Loop \
		    	} \
		    	break; \
		    case 8: \
		    	while (hTmp--) { \
			    GetBits \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) Step \
			    StoreBits(6) Step StoreBits(7) \
			    Loop \
		    	} \
		    	break; \
		    }

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
    register int	    xoff1, xoff2, xoff3, xoff4;
    register unsigned long  *char1, *char2, *char3, *char4;
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
    int			widthGlyph4;

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
    widthGlyph4 = widthGlyph << 2;
    pdstBase += y * widthDst;
    while (nglyph)
    {
	xoff1 = x & 0x3;
	char1 = (unsigned long *) (pglyphBase + (*ppci++)->byteOffset);
	hTmp = h;
	dstLine = pdstBase + (x >> 2);
	if (nglyph > 3 && xoff1 + widthGlyph4 < 36)
	{
	    nglyph -= 4;
	    xoff2 = xoff1 + widthGlyph;
	    xoff3 = xoff2 + widthGlyph;
	    xoff4 = xoff3 + widthGlyph;
	    char2 = (unsigned long *) (pglyphBase + (*ppci++)->byteOffset);
	    char3 = (unsigned long *) (pglyphBase + (*ppci++)->byteOffset);
	    char4 = (unsigned long *) (pglyphBase + (*ppci++)->byteOffset);
	    oldRightChar = char4;
	    dst = dstLine;
	    if (xoff1)
	    {
		ew = ((widthGlyph4 - (4 - xoff1)) >> 2) + 1;
	    	if (!leftChar)
	    	{
		    leftMask = cfbendtab[xoff1];
		    rightMask = cfbstarttab[xoff1];

#define Step		NextFourBits(c);
#define Loop		dst += widthDst;
#define StoreBits0	dst[0] = dst[0] & leftMask | GetFourPixels(c) & rightMask;

#if (BITMAP_BIT_ORDER == MSBFirst)
#define StoreBits(o)	dst[o] = GetFourPixels(c);
#define FirstStep	Step
#else
#define StoreBits(o)	dst[o] = *((unsigned long *) (((char *) cfb8Pixels) + (c & 0x3c)));
#define FirstStep	c = BitLeft (c, 2);
#endif

#define GetBits	    c = BitRight (*char1++, xoff1) | \
			BitRight (*char2++, xoff2) | \
			BitRight (*char3++, xoff3) | \
			BitRight (*char4++, xoff4);

		    SwitchEm

#undef GetBits
#undef StoreBits0

	    	}
	    	else
	    	{
		    lshift = widthGlyph - xoff1;

#define StoreBits0  dst[0] = GetFourPixels(c);

#define GetBits	    c = BitLeft  (*leftChar++, lshift) | \
			BitRight (*char1++, xoff1) | \
			BitRight (*char2++, xoff2) | \
			BitRight (*char3++, xoff3) | \
			BitRight (*char4++, xoff4);
		    SwitchEm
#undef GetBits

	    	}
	    }
	    else
	    {
	    	ew = widthGlyph;    /* widthGlyph4 >> 2 */

#define GetBits	    c = *char1++ | \
			BitRight (*char2++, xoff2) | \
			BitRight (*char3++, xoff3) | \
			BitRight (*char4++, xoff4);
	    	SwitchEm
#undef GetBits

	    }
	    x += widthGlyph4;
	}
	else
	{
	    nglyph--;
	    oldRightChar = char1;
	    dst = dstLine;
	    if (xoff1)
	    {
		ew = ((widthGlyph - (4 - xoff1)) >> 2) + 1;
	    	if (!leftChar)
	    	{
		    leftMask = cfbendtab[xoff1];
		    rightMask = cfbstarttab[xoff1];
#undef StoreBits0
#define StoreBits0	dst[0] = dst[0] & leftMask | GetFourPixels(c) & rightMask;
#define GetBits		c = BitRight (*char1++, xoff1);

		    SwitchEm
#undef GetBits
#undef StoreBits0

	    	}
	    	else
	    	{
		    lshift = widthGlyph - xoff1;
#define GetBits	    c = BitLeft (*leftChar++, lshift) | \
			BitRight (*char1++, xoff1);

#define StoreBits0  dst[0] = GetFourPixels(c);

		    SwitchEm
#undef GetBits
	    	}
	    }
	    else
	    {
	    	ew = widthGlyph >> 2;
#define GetBits	c = *char1++;

		SwitchEm
#undef GetBits

	    }
	    x += widthGlyph;
	}
	leftChar = oldRightChar;
    }
    /*
     * draw the tail of the last character
     */
    xoff1 = x & 3;
    if (xoff1)
    {
	rightMask = cfbstarttab[xoff1];
	leftMask = cfbendtab[xoff1];
	lshift = widthGlyph - xoff1;
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
