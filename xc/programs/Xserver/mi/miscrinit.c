/*

Copyright 1990 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

/* $XConsortium: miscrinit.c,v 5.6 93/07/16 15:46:09 dpw Exp $ */

#include "X.h"
#include "servermd.h"
#include "misc.h"
#include "mi.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "mibstore.h"
#include "dix.h"

/* We use this structure to propogate some information from miScreenInit to
 * miCreateScreenResources.  miScreenInit allocates the structure, fills it
 * in, and puts it into pScreen->devPrivate.  miCreateScreenResources 
 * extracts the info and frees the structure.  We could've accomplished the
 * same thing by adding fields to the screen structure, but they would have
 * ended up being redundant, and would have exposed this mi implementation
 * detail to the whole server.
 */

typedef struct
{
    pointer pbits; /* pointer to framebuffer */
    int width;    /* delta to add to a framebuffer addr to move one row down */
} miScreenInitParmsRec, *miScreenInitParmsPtr;


/* this plugs into pScreen->ModifyPixmapHeader */
Bool
miModifyPixmapHeader(pPixmap, width, height, depth, bitsPerPixel, devKind,
		     pPixData)
    PixmapPtr   pPixmap;
    int		width;
    int		height;
    int		depth;
    int		bitsPerPixel;
    int		devKind;
    pointer     pPixData;
{
    if (!pPixmap)
	return FALSE;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = bitsPerPixel;
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = devKind;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = pPixData;
    return TRUE;
}


/*ARGSUSED*/
Bool
miCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    return ((*pScreen->DestroyPixmap)((PixmapPtr)pScreen->devPrivate));
}

/* With the introduction of pixmap privates, the "screen pixmap" can no
 * longer be created in miScreenInit, since all the modules that could
 * possibly ask for pixmap private space have not been initialized at
 * that time.  pScreen->CreateScreenResources is called after all
 * possible private-requesting modules have been inited; we create the
 * screen pixmap here.
 */
Bool
miCreateScreenResources(pScreen)
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
		    pScreen->height, pScreen->rootDepth, pScreen->rootDepth,
		    PixmapBytePad(pScrInitParms->width, pScreen->rootDepth),
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

Bool
miScreenDevPrivateInit(pScreen, width, pbits)
    register ScreenPtr pScreen;
    int width;
    pointer pbits;
{
    miScreenInitParmsPtr pScrInitParms;

    /* Stash pbits and width in a short-lived miScreenInitParmsRec attached
     * to the screen, until CreateScreenResources can put them in the
     * screen pixmap.
     */
    pScrInitParms = (miScreenInitParmsPtr)xalloc(sizeof(miScreenInitParmsRec));
    if (!pScrInitParms)
	return FALSE;
    pScrInitParms->pbits = pbits;
    pScrInitParms->width = width;
    pScreen->devPrivate = (pointer)pScrInitParms;
    return TRUE;
}

/*
 * If you pass in bsfuncs, then you must preinitialize the missing
 * screen procs before calling miScreenInit, so that the backing store
 * code can correctly wrap them.
 */

Bool
miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
	     rootDepth, numDepths, depths, rootVisual, numVisuals, visuals,
	     bsfuncs)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bits */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int rootDepth;		/* depth of root window */
    int numDepths;		/* number of depths supported */
    DepthRec *depths;		/* supported depths */
    VisualID rootVisual;	/* root visual */
    int numVisuals;		/* number of visuals supported */
    VisualRec *visuals;		/* supported visuals */
    miBSFuncPtr	bsfuncs;	/* backing store functions */
{
    pScreen->width = xsize;
    pScreen->height = ysize;
    pScreen->mmWidth = (xsize * 254) / (dpix * 10);
    pScreen->mmHeight = (ysize * 254) / (dpiy * 10);
    pScreen->numDepths = numDepths;
    pScreen->rootDepth = rootDepth;
    pScreen->allowedDepths = depths;
    pScreen->rootVisual = rootVisual;
    /* defColormap */
    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 1;
    pScreen->backingStoreSupport = Always;
    pScreen->saveUnderSupport = NotUseful;
    /* whitePixel, blackPixel */
    pScreen->ModifyPixmapHeader = miModifyPixmapHeader;
    pScreen->CreateScreenResources = miCreateScreenResources;
    pScreen->numVisuals = numVisuals;
    pScreen->visuals = visuals;
    if (width)
    {
#ifdef MITSHM
	ShmRegisterFbFuncs(pScreen);
#endif
	pScreen->CloseScreen = miCloseScreen;
    }
    /* else CloseScreen */
    /* QueryBestSize, SaveScreen, GetImage, GetSpans */
    pScreen->PointerNonInterestBox = (void (*)()) 0;
    pScreen->SourceValidate = (void (*)()) 0;
    /* CreateWindow, DestroyWindow, PositionWindow, ChangeWindowAttributes */
    /* RealizeWindow, UnrealizeWindow */
    pScreen->ValidateTree = miValidateTree;
    pScreen->PostValidateTree = (void (*)()) 0;
    pScreen->WindowExposures = miWindowExposures;
    /* PaintWindowBackground, PaintWindowBorder, CopyWindow */
    pScreen->ClearToBackground = miClearToBackground;
    pScreen->ClipNotify = (void (*)()) 0;
    /* CreatePixmap, DestroyPixmap */
    /* RealizeFont, UnrealizeFont */
    /* CreateGC */
    /* CreateColormap, DestroyColormap, InstallColormap, UninstallColormap */
    /* ListInstalledColormaps, StoreColors, ResolveColor */
    pScreen->RegionCreate = miRegionCreate;
    pScreen->RegionInit = miRegionInit;
    pScreen->RegionCopy = miRegionCopy;
    pScreen->RegionDestroy = miRegionDestroy;
    pScreen->RegionUninit = miRegionUninit;
    pScreen->Intersect = miIntersect;
    pScreen->Union = miUnion;
    pScreen->Subtract = miSubtract;
    pScreen->Inverse = miInverse;
    pScreen->RegionReset = miRegionReset;
    pScreen->TranslateRegion = miTranslateRegion;
    pScreen->RectIn = miRectIn;
    pScreen->PointInRegion = miPointInRegion;
    pScreen->RegionNotEmpty = miRegionNotEmpty;
    pScreen->RegionEmpty = miRegionEmpty;
    pScreen->RegionExtents = miRegionExtents;
    pScreen->RegionAppend = miRegionAppend;
    pScreen->RegionValidate = miRegionValidate;
    /* BitmapToRegion */
    pScreen->RectsToRegion = miRectsToRegion;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;
    pScreen->BlockHandler = (void (*)())NoopDDA;
    pScreen->WakeupHandler = (void (*)())NoopDDA;
    pScreen->blockData = (pointer)0;
    pScreen->wakeupData = (pointer)0;
    if (bsfuncs)
	miInitializeBackingStore (pScreen, bsfuncs);

    return miScreenDevPrivateInit(pScreen, width, pbits);
}
