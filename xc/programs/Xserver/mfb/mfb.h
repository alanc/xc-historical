/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
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
/* $XConsortium: mfb.h,v 5.3 89/07/12 17:17:54 keith Exp $ */
/* Monochrome Frame Buffer definitions 
   written by drewry, september 1986
*/
#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "colormap.h"
#include "miscstruct.h"
#include "mibstore.h"

extern int InverseAlu[];

extern Bool mfbScreenInit();
extern Bool mfbCloseScreen();
extern Bool mfbCreateDefColormap();
extern void mfbQueryBestSize();
extern Bool mfbCreateWindow();
extern Bool mfbPositionWindow();
extern Bool mfbChangeWindowAttributes();
extern Bool mfbMapWindow();
extern Bool mfbUnmapWindow();
extern Bool mfbDestroyWindow();

extern Bool mfbRealizeFont();
extern Bool mfbUnrealizeFont();
extern Bool mfbScreenSaver();
extern Bool mfbCreateGC();
extern int  mfbReduceRop();

extern PixmapPtr mfbCreatePixmap();
extern Bool mfbDestroyPixmap();

extern void mfbCopyWindow();

extern void mfbSaveAreas();
extern void mfbRestoreAreas();

/* window painter */
extern void mfbPaintWindow();

/* rectangle painters */
extern void mfbSolidWhiteArea();
extern void mfbStippleWhiteArea();
extern void mfbSolidBlackArea();
extern void mfbStippleBlackArea();
extern void mfbSolidInvertArea();
extern void mfbStippleInvertArea();
extern void mfbTileArea32();

extern void mfbPolyFillRect();
extern RegionPtr mfbCopyArea();
extern void mfbPolyPoint();
extern RegionPtr mfbCopyPlane();

extern void mfbSetSpans();
extern unsigned int *mfbGetSpans();
extern void mfbWhiteSolidFS();
extern void mfbBlackSolidFS();
extern void mfbInvertSolidFS();
extern void mfbWhiteStippleFS();
extern void mfbBlackStippleFS();
extern void mfbInvertStippleFS();
extern void mfbTileFS();
extern void mfbUnnaturalTileFS();
extern void mfbUnnaturalStippleFS();

extern void mfbGetImage();
extern void mfbPutImage();

extern void mfbLineSS();	/* solid single-pixel wide line */
				/* calls mfb{Bres|Horz|Vert}S() */
extern void mfbDashLine();	/* dashed zero-width line */
extern void mfbImageText8();
extern void mfbImageText16();
extern int mfbPolyText16();
extern int mfbPolyText8();
extern PixmapPtr mfbCopyPixmap();
extern RegionPtr mfbPixmapToRegion();
extern void mfbPushPixels();

/* text for glyphs <= 32 bits wide */
extern void mfbImageGlyphBltWhite();
extern void mfbImageGlyphBltBlack();
extern void mfbPolyGlyphBltWhite();
extern void mfbPolyGlyphBltBlack();
extern void mfbPolyGlyphBltInvert();

/* text for terminal emulator fonts */
extern void mfbTEGlyphBltWhite();	/* fg = 1, bg = 0 */
extern void mfbTEGlyphBltBlack();	/* fg = 0, bg = 1 */

extern int mfbListInstalledColormaps();
extern void mfbInstallColormap();
extern void mfbUninstallColormap();
extern Bool mfbCreateColormap();
extern void mfbDestroyColormap();
extern void mfbResolveColor();

extern void mfbCopyGCDest();

/*
   private filed of pixmap
   pixmap.devPrivate = (unsigned int *)pointer_to_bits
   pixmap.devKind = width_of_pixmap_in_bytes

   private field of screen
   a pixmap, for which we allocate storage.  devPrivate is a pointer to
the bits in the hardware framebuffer.  note that devKind can be poked to
make the code work for framebuffers that are wider than their
displayable screen (e.g. the early vsII, which displayed 960 pixels
across, but was 1024 in the hardware.)  this is left to the
code that calls mfbScreenInit(), rather than having mfbScreenInit()
take yet another parameter.

   private field of GC 
	pAbsClientRegion is always a real region, although perhaps
an empty one.
	Freeing pCompositeClip is done based on the value of
freeCompClip; if freeCompClip is not carefully maintained, we will end
up losing storage or freeing something that isn't ours.
*/

typedef struct {
    unsigned char	rop;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip;	/* rop for opaque stipple */
    unsigned char	ropFillArea;	/*  == alu, rop, or ropOpStip */
    short	fExpose;		/* callexposure handling ? */
    short	freeCompClip;
    PixmapPtr	pRotatedTile;		/* tile/stipple  rotated to align */
    PixmapPtr	pRotatedStipple;	/* with window and using offsets */
    RegionPtr	pAbsClientRegion;	/* client region in screen coords */
    RegionPtr	pCompositeClip;		/* free this based on freeCompClip
					   flag rather than NULLness */
    void 	(* FillArea)();		/* fills regions; look at the code */
    PixmapPtr   *ppPixmap;		/* points to the pixmapPtr to
					   use for tiles and stipples */
    } mfbPrivGC;
typedef mfbPrivGC	*mfbPrivGCPtr;

extern int  mfbGCPrivateIndex;		/* index into GC private array */
extern int  mfbWindowPrivateIndex;	/* index into Window private array */

/* freeCompositeClip values */
#define REPLACE_CC	0		/* compsite clip is a copy of a
					   pointer, so it doesn't need to 
					   be freed; just overwrite it.
					   this happens if there is no
					   client clip and the gc has
					   ClipByChildren in it.
					*/
#define FREE_CC		1		/* composite clip is a real
					   region that we need to free
					*/

/* private field of window */
typedef struct {
    int		fastBorder;	/* non-zero if border tile is 32 bits wide */
    int		fastBackground;
    DDXPointRec	oldRotate;
    PixmapPtr	pRotatedBackground;
    PixmapPtr	pRotatedBorder;
    } mfbPrivWin;

/* precomputed information about each glyph for GlyphBlt code.
   this saves recalculating the per glyph information for each
box.
*/
typedef struct _pos{
    int xpos;		/* xposition of glyph's origin */
    int xchar;		/* x position mod 32 */
    int leftEdge;
    int rightEdge;
    int topEdge;
    int bottomEdge;
    unsigned int *pdstBase;	/* longword with character origin */
    int widthGlyph;	/* width in bytes of this glyph */
} TEXTPOS;

/* reduced raster ops for mfb */
#define RROP_BLACK	GXclear
#define RROP_WHITE	GXset
#define RROP_NOP	GXnoop
#define RROP_INVERT	GXinvert

/* out of clip region codes */
#define OUT_LEFT 0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

/* major axis for bresenham's line */
#define X_AXIS	0
#define Y_AXIS	1

/* optimization codes for FONT's devPrivate field */
#define FT_VARPITCH	0
#define FT_SMALLPITCH	1
#define FT_FIXPITCH	2

/* macros for mfbbitblt.c, mfbfillsp.c
   these let the code do one switch on the rop per call, rather
than a switch on the rop per item (span or rectangle.)
*/

#define fnCLEAR(src, dst)	(0)
#define fnAND(src, dst) 	(src & dst)
#define fnANDREVERSE(src, dst)	(src & ~dst)
#define fnCOPY(src, dst)	(src)
#define fnANDINVERTED(src, dst)	(~src & dst)
#define fnNOOP(src, dst)	(dst)
#define fnXOR(src, dst)		(src ^ dst)
#define fnOR(src, dst)		(src | dst)
#define fnNOR(src, dst)		(~(src | dst))
#define fnEQUIV(src, dst)	(~src ^ dst)
#define fnINVERT(src, dst)	(~dst)
#define fnORREVERSE(src, dst)	(src | ~dst)
#define fnCOPYINVERTED(src, dst)(~src)
#define fnORINVERTED(src, dst)	(~src | dst)
#define fnNAND(src, dst)	(~(src & dst))
#define fnSET(src, dst)		(~0)

#ifndef PURDUE
/* Binary search to figure out what to do for the raster op.  It may
 * do 5 comparisons, but at least it does no function calls 
 * Special cases copy because it's so frequent 
 */
#define DoRop(alu, src, dst) \
( ((alu) == GXcopy) ? (src) : \
    (((alu) >= GXnor) ? \
     (((alu) >= GXcopyInverted) ? \
       (((alu) >= GXnand) ? \
         (((alu) == GXnand) ? ~((src) & (dst)) : ~0) : \
         (((alu) == GXcopyInverted) ? ~(src) : (~(src) | (dst)))) : \
       (((alu) >= GXinvert) ? \
	 (((alu) == GXinvert) ? ~(dst) : ((src) | ~(dst))) : \
	 (((alu) == GXnor) ? ~((src) | (dst)) : (~(src) ^ (dst)))) ) : \
     (((alu) >= GXandInverted) ? \
       (((alu) >= GXxor) ? \
	 (((alu) == GXxor) ? ((src) ^ (dst)) : ((src) | (dst))) : \
	 (((alu) == GXnoop) ? (dst) : (~(src) & (dst)))) : \
       (((alu) >= GXandReverse) ? \
	 (((alu) == GXandReverse) ? ((src) & ~(dst)) : (src)) : \
	 (((alu) == GXand) ? ((src) & (dst)) : 0)))  ) )
#else  /* PURDUE */
/*  Using a "switch" statement is much faster in most cases
 *  since the compiler can do a look-up table or multi-way branch
 *  instruction, depending on the architecture.  The result on
 *  A Sun 3/50 is at least 2.5 times faster, assuming a uniform
 *  distribution of RasterOp operation types.
 *
 *  However, doing some profiling on a running system reveals
 *  GXcopy is the operation over 99.5% of the time and
 *  GXcopy is the next most frequent (about .4%), so we make special
 *  checks for those first.
 *
 *  Note that this requires a change to the "calling sequence"
 *  since we can't engineer a "switch" statement to have an lvalue.
 */
#define DoRop(result, alu, src, dst) \
{ \
    if (alu == GXcopy) \
	result = fnCOPY (src, dst); \
    else if (alu == GXxor) \
        result = fnXOR (src, dst); \
    else \
	switch (alu) \
	{ \
	  case GXclear: \
	    result = fnCLEAR (src, dst); \
	    break; \
	  case GXand: \
	    result = fnAND (src, dst); \
	    break; \
	  case GXandReverse: \
	    result = fnANDREVERSE (src, dst); \
	    break; \
	  case GXandInverted: \
	    result = fnANDINVERTED (src, dst); \
	    break; \
	  case GXnoop: \
	    result = fnNOOP (src, dst); \
	    break; \
	  case GXor: \
	    result = fnOR (src, dst); \
	    break; \
	  case GXnor: \
	    result = fnNOR (src, dst); \
	    break; \
	  case GXequiv: \
	    result = fnEQUIV (src, dst); \
	    break; \
	  case GXinvert: \
	    result = fnINVERT (src, dst); \
	    break; \
	  case GXorReverse: \
	    result = fnORREVERSE (src, dst); \
	    break; \
	  case GXcopyInverted: \
	    result = fnCOPYINVERTED (src, dst); \
	    break; \
	  case GXorInverted: \
	    result = fnORINVERTED (src, dst); \
	    break; \
	  case GXnand: \
	    result = fnNAND (src, dst); \
	    break; \
	  case GXset: \
	    result = fnSET (src, dst); \
	    break; \
	} \
}
#endif  /* PURDUE */


#define DoRRop(alu, src, dst) \
(((alu) == RROP_BLACK) ? ((dst) & ~(src)) : \
 ((alu) == RROP_WHITE) ? ((dst) | (src)) : \
 ((alu) == RROP_INVERT) ? ((dst) ^ (src)) : \
  (dst))

#ifdef PURDUE
/* A generalized form of a x4 Duff's Device */
#define Duff(counter, block) { \
  while (counter >= 4) {\
     { block; } \
     { block; } \
     { block; } \
     { block; } \
     counter -= 4; \
  } \
     switch (counter & 3) { \
     case 3:	{ block; } \
     case 2:	{ block; } \
     case 1:	{ block; } \
     case 0: \
     counter = 0; \
   } \
}
#endif  /* PURDUE */
