/* $XConsortium: ppcIO.c,v 1.1 94/03/28 21:36:57 dpw Exp $ */
/*

Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright IBM Corporation 1987,1988,1989
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that 
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/


#include "X.h"
#include "resource.h"
#include "scrnintstr.h"
#include "servermd.h"	/* GJA */
#include "mi.h" /* GJA */

#include "compiler.h"	/* rvb */
#include "vgaVideo.h"

#include "ibmTrace.h"
#include "windowstr.h"
#include "ppc.h"

#include "mistruct.h"
#include "mfb.h"
#include "mi.h"

static PixmapPtr BogusPixmap = (PixmapPtr) 1 ;
extern ScreenRec vgaScreenRec ; /* Forward Declaration Here */

PixmapFormatRec	vgaFormats[] = { { 4, 8, 32 } } ;

VisualRec vgaVisuals[] = {
	{
	0,			/* unsigned long	vid */
	PseudoColor,		/* short       class */
	6,			/* short       bitsPerRGBValue */
	1 << VGA_MAXPLANES,	/* short	ColormapEntries */
	VGA_MAXPLANES,		/* short	nplanes */
	0,			/* unsigned long	redMask */
	0,			/* unsigned long	greenMask */
	0,			/* unsigned long	blueMask */
	0,			/* int		offsetRed */
	0,			/* int		offsetGreen */
	0			/* int		offsetBlue */
	}
} ;

#define NUM_VISUALS  (sizeof vgaVisuals/sizeof (VisualRec))

unsigned long int vgaDepthVIDs[NUM_VISUALS];

DepthRec vgaDepths[] = {
/*	depth		numVid	vids */
    {	1,		0,	NULL	},
    {	VGA_MAXPLANES,	NUM_VISUALS,	vgaDepthVIDs }
} ;

#define NUM_DEPTHS  (sizeof vgaDepths/sizeof (DepthRec))

NeverCalled()
{
	ErrorF("NeverCalled was nevertheless called\n");
	abort();
}

Bool ppcInitializeColormap();	/* GJAHACK */

miBSFuncRec ppcBSFuncRec = {
    ppcSaveAreas,
    ppcRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};


/* Global Variables */
int BYTES_PER_LINE, MAX_ROW, MAX_COLUMN;

/*ARGSUSED*/
Bool
vgaScreenClose( index, pScreen )
int	index;
ScreenPtr pScreen;
{
	pScreen->defColormap = 0 ;
	return TRUE;
}


GCPtr sampleGCperDepth[MAXFORMATS+1] = { 0 };
PixmapPtr samplePixmapPerDepth[1] = { 0 };

/* GJA -- Took this from miscrinit.c.
 * We want that devKind contains the distance in bytes between two scanlines.
 * The computation that mi does is not appropriate for planar VGA.
 * Therefore we provide here our own routine.
 */

/* GJA -- WARNING: this is an internal structure declaration, taken from
 * miscrinit.c
 */
typedef struct
{
    pointer pbits; /* pointer to framebuffer */
    int width;    /* delta to add to a framebuffer addr to move one row down */
} miScreenInitParmsRec, *miScreenInitParmsPtr;

/* With the introduction of pixmap privates, the "screen pixmap" can no
 * longer be created in miScreenInit, since all the modules that could
 * possibly ask for pixmap private space have not been initialized at
 * that time.  pScreen->CreateScreenResources is called after all
 * possible private-requesting modules have been inited; we create the
 * screen pixmap here.
 */
Bool
v16CreateScreenResources(pScreen)
    ScreenPtr pScreen;
{
    miScreenInitParmsPtr pScrInitParms;
    pointer value;

    pScrInitParms = (miScreenInitParmsPtr)pScreen->devPrivate;

    /* if width is non-zero, pScreen->devPrivate will be a pixmap
     * else it will just take the value pbits
     */
    if (pScrInitParms->width)
    {
	PixmapPtr pPixmap;

	/* create a pixmap with no data, then redirect it to point to
	 * the screen
	 */
	pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, pScreen->rootDepth);
	if (!pPixmap)
	    return FALSE;

	if (!(*pScreen->ModifyPixmapHeader)(pPixmap, pScreen->width,
		    pScreen->height, pScreen->rootDepth, 8 /* bits per pixel */,
/* GJA: was 	    PixmapBytePad(pScrInitParms->width, pScreen->rootDepth), */
#define BITS_PER_BYTE_SHIFT 3
		    pScrInitParms->width >> BITS_PER_BYTE_SHIFT,
		    pScrInitParms->pbits))
	    return FALSE;
	value = (pointer)pPixmap;
    }
    else
    {
	value = pScrInitParms->pbits;
    }
    xfree(pScreen->devPrivate); /* freeing miScreenInitParmsRec */
    pScreen->devPrivate = value; /* pPixmap or pbits */
    return TRUE;
}


void
Init16Output( pScreen, virtx, virty )
    ScreenPtr pScreen;
    int virtx, virty;
{
  extern int defaultColorVisualClass;
#ifdef UNBANKED_VGA16
  extern pointer vgaBase; /* GJAHACK */
#else
  extern pointer vgaVirtBase; /* GJAHACK */
#endif

  /*
   * patch the original structures
   */
  BYTES_PER_LINE = virtx / 8;
  MAX_ROW       = virty - 1;
  MAX_COLUMN    = virtx - 1;

  if ( defaultColorVisualClass < 0 ) {
    vgaVisuals[0].class = PseudoColor;
  } else {
    vgaVisuals[0].class = defaultColorVisualClass;
  }

  pScreen-> id = 0;
  pScreen->defColormap = FakeClientID(0);
  pScreen-> whitePixel = VGA_WHITE_PIXEL;
  pScreen-> blackPixel = VGA_BLACK_PIXEL;
  pScreen-> rgf = 0;
  *(pScreen-> GCperDepth) = *(sampleGCperDepth);
  *(pScreen-> PixmapPerDepth) = *(samplePixmapPerDepth);
  pScreen-> numVisuals = sizeof vgaVisuals/sizeof (VisualRec);
  pScreen-> visuals = &vgaVisuals[0];
  pScreen-> CloseScreen = vgaScreenClose;
  pScreen-> QueryBestSize = ppcQueryBestSize;
  pScreen-> GetImage = ppcGetImage;
  pScreen-> GetSpans = ppcGetSpans;
  pScreen-> CreateWindow = ppcCreateWindowForXYhardware;
  pScreen-> DestroyWindow = ppcDestroyWindow;
  pScreen-> PositionWindow = ppcPositionWindow;
  pScreen-> ChangeWindowAttributes = mfbChangeWindowAttributes;
  pScreen-> RealizeWindow = mfbMapWindow;
  pScreen-> UnrealizeWindow = mfbUnmapWindow;
  pScreen-> PaintWindowBackground = ppcPaintWindow;
  pScreen-> PaintWindowBorder = ppcPaintWindow;
  pScreen-> CopyWindow = ppcCopyWindow;
  pScreen-> CreatePixmap = ppcCreatePixmap;
  pScreen-> DestroyPixmap = mfbDestroyPixmap;
  pScreen-> SaveDoomedAreas = (SaveDoomedAreasProcPtr)NoopDDA;
  pScreen-> RestoreAreas = (RegionPtr(*)())ppcRestoreAreas;
  pScreen-> ExposeCopy = (ExposeCopyProcPtr)NoopDDA;
  pScreen-> TranslateBackingStore = (RegionPtr(*)())NoopDDA;
  pScreen-> ClearBackingStore = (RegionPtr(*)())NoopDDA;
  pScreen-> DrawGuarantee = (DrawGuaranteeProcPtr)NoopDDA;
  pScreen-> RealizeFont = mfbRealizeFont;
  pScreen-> UnrealizeFont = mfbUnrealizeFont;
  pScreen-> CreateGC = ppcCreateGC;
  pScreen-> CreateColormap = (Bool (*)())ppcInitializeColormap;
  pScreen-> DestroyColormap = (void (*)())NoopDDA;
  pScreen-> ResolveColor = ppcResolveColor;
  pScreen-> BitmapToRegion = mfbPixmapToRegion;

  if (!mfbAllocatePrivates(pScreen, (int*)NULL, (int*)NULL))
	return ;

#ifdef UNBANKED_VGA16
  miScreenInit(pScreen, vgaBase, virtx, virty, 75, 75, virtx,
#else
  miScreenInit(pScreen, vgaVirtBase, virtx, virty, 75, 75, virtx,
#endif
	VGA_MAXPLANES, NUM_DEPTHS, vgaDepths, 0 /* See above */,
	NUM_VISUALS, vgaVisuals, &ppcBSFuncRec);
  /* GJA -- Now we override the supplied default: */
  pScreen -> CreateScreenResources = v16CreateScreenResources;

  mfbRegisterCopyPlaneProc(pScreen,miCopyPlane); /* GJA -- R4->R5 */
}
