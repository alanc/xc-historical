/* $XConsortium: mibstore.c,v 5.13 89/07/13 19:23:13 keith Exp $ */
/***********************************************************
Copyright 1987 by the Regents of the University of California
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission.  

The University of California makes no representations about the suitability
of this software for any purpose.  It is provided "as is" without express or
implied warranty.

******************************************************************/

#define NEED_EVENTS
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "dixstruct.h"		/* For requestingClient */
#include "mi.h"
#include "mibstorest.h"

/*-
 * NOTES ON USAGE:
 *
 * The functions in this file implement a machine-independent backing-store
 * scheme. To use it, the output library must do the following:
 *	- Provide a SaveAreas function that takes a destination pixmap, a
 *	    region of the areas to save (in the pixmap's coordinate system)
 *	    and the screen origin of the region. It should copy the areas from
 *	    the screen into the pixmap.
 *	- Provide a RestoreAreas function that takes a source pixmap, a region
 *	    of the areas to restore (in the screen's coordinate system) and the
 *	    origin of the pixmap on the screen. It should copy the areas from
 *	    the pixmap into the screen.
 *	- Provide a SetClipmaskRgn function that takes a gc and a region
 *	    and merges the region into any CT_PIXMAP client clip that
 *	    is specified in the GC.  This routine is only needed if
 *	    miValidateBackingStore will see CT_PIXMAP clip lists; not
 *	    true for any of the sample servers (which convert the PIXMAP
 *	    clip lists into CT_REGION clip lists; an expensive but simple
 *	    to code option).
 *	- The GetSpans function must call miBSGetSpans at the end of its
 *	    operation, passing in the source drawable, a pixmap via which
 *	    spans from the backing store may be drawn into those fetched from
 *	    the screen, followed by the other four arguments GetSpans received
 *	    and the array of widths with each width padded to the actual width
 *	    of the span in the pixmap (see the declaration for miBSGetSpans,
 *	    below). This should only be done if the source drawable is a
 *	    window and backingStore is not NotUseful.
 *	- The GetImage function should call miBSGetImage at its end, passing
 *	    the source drawable, a pixmap through which the extracted image
 *	    may be modified, followed by all the other arguments, in their
 *	    original order, except for the image memory's address. Once again
 *	    this should only be done if the source drawable is a window with
 *	    backingStore enabled.
 *	- The function placed in a window's ClearToBackground vector must call
 *	    pWin->backStorage->ClearToBackground with the window, followed by
 *	    the window-relative x and y coordinates, followed by the width and
 *	    height of the area to be cleared, followed by the generateExposures
 *	    flag. This has been taken care of in miClearToBackground.
 *	- Whatever determines GraphicsExpose events for the CopyArea and
 *	    CopyPlane requests should call pWin->backStorage->ExposeCopy
 *	    with the source and destination drawables, the GC used, a source-
 *	    window-relative region of exposed areas, the source and destination
 *	    coordinates and the bitplane copied, if CopyPlane, or 0, if
 *	    CopyArea.
 *
 * JUSTIFICATION
 *    This is a cross between saving everything and just saving the
 * obscued areas (as in Pike's layers.)  This method has the advantage
 * of only doing each output operation once per pixel, visible or
 * invisible, and avoids having to do all the crufty storage
 * management of keeping several separate rectangles.  Since the
 * ddx layer ouput primitives are required to draw through clipping
 * rectangles anyway, sending multiple drawing requests for each of
 * several rectangles isn't necessary.  (Of course, it could be argued
 * that the ddx routines should just take one rectangle each and
 * get called multiple times, but that would make taking advantage of
 * smart hardware harder, and probably be slower as well.)
 */

#define SETUP_BACKING_TERSE(pGC) \
    miBSGCPtr	pGCPrivate = (miBSGCPtr)(pGC)->devPrivates[miBSGCIndex].ptr; \
    GCFuncs	*oldFuncs = pGC->funcs;

#define SETUP_BACKING(pDrawable,pGC) \
    DrawablePtr	  pBackingDrawable = (DrawablePtr) \
    	((miBSWindowPtr)((WindowPtr)(pDrawable))->backStorage)->pBackingPixmap; \
    SETUP_BACKING_TERSE(pGC) \
    GCPtr	pBackingGC = pGCPrivate->pBackingGC;

#define SETUP_BACKING_VERBOSE(pDrawable,pGC) \
    miBSWindowPtr pBackingStore = \
    	(miBSWindowPtr)((WindowPtr)(pDrawable))->backStorage; \
    SETUP_BACKING(pDrawable, pGC)

#define PROLOGUE(pGC) \
    pGC->ops = pGCPrivate->wrapOps;\
    pGC->funcs = pGCPrivate->wrapFuncs

#define EPILOGUE(pGC) \
    (pGC->ops = &miBSGCOps), \
    (pGC->funcs = oldFuncs)
   
static void	    miCreateBSPixmap();
static void	    miDestroyBSPixmap();
static void	    miTileVirtualBS();
static void	    miBSAllocate(), miBSFree();

/*
 * wrappers for screen funcs
 */

static int  miBSScreenIndex = -1;

static Bool	    miBSCloseScreen();
static void	    miBSGetImage();
static unsigned int *miBSGetSpans();
static Bool	    miBSChangeWindowAttributes();
static Bool	    miBSCreateGC();
static Bool	    miBSDestroyWindow();

/*
 * backing store screen functions
 */

static void	    miBSSaveDoomedAreas();
static RegionPtr    miBSRestoreAreas();
static void	    miBSExposeCopy(),		miBSTranslateBackingStore();
static void	    miBSClearBackingStore(),	miBSDrawGuarantee();

/*
 * wrapper vectors for GC funcs and ops
 */

static int  miBSGCIndex = -1;

static void miBSValidateGC (),	miBSCopyGC (),	    miBSDestroyGC();
static void miBSChangeGC();
static void miBSChangeClip(),	miBSDestroyClip(),  miBSCopyClip();

static GCFuncs	miBSGCFuncs = {
    miBSValidateGC,
    miBSChangeGC,
    miBSCopyGC,
    miBSDestroyGC,
    miBSChangeClip,
    miBSDestroyClip,
    miBSCopyClip,
};

static void	    miBSFillSpans(),	miBSSetSpans(),	    miBSPutImage();
static RegionPtr    miBSCopyArea(),	miBSCopyPlane();
static void	    miBSPolyPoint(),	miBSPolylines(),    miBSPolySegment();
static void	    miBSPolyRectangle(),miBSPolyArc(),	    miBSFillPolygon();
static void	    miBSPolyFillRect(),	miBSPolyFillArc();
static int	    miBSPolyText8(),	miBSPolyText16();
static void	    miBSImageText8(),	miBSImageText16();
static void	    miBSImageGlyphBlt(),miBSPolyGlyphBlt();
static void	    miBSPushPixels(),	miBSLineHelper();

static GCOps miBSGCOps = {
    miBSFillSpans,	miBSSetSpans,	    miBSPutImage,	
    miBSCopyArea,	miBSCopyPlane,	    miBSPolyPoint,
    miBSPolylines,	miBSPolySegment,    miBSPolyRectangle,
    miBSPolyArc,	miBSFillPolygon,    miBSPolyFillRect,
    miBSPolyFillArc,	miBSPolyText8,	    miBSPolyText16,
    miBSImageText8,	miBSImageText16,    miBSImageGlyphBlt,
    miBSPolyGlyphBlt,	miBSPushPixels,	    miBSLineHelper,
};

#define FUNC_PROLOGUE(pGC, pPriv) \
    ((pGC)->funcs = pPriv->wrapFuncs),\
    ((pGC)->ops = pPriv->wrapOps)

#define FUNC_EPILOGUE(pGC, pPriv) \
    ((pGC)->funcs = &miBSGCFuncs),\
    ((pGC)->ops = &miBSGCOps)

/*
 * every GC in the server is initially wrapped with these
 * "cheap" functions.  This allocates no memory and is used
 * to discover GCs used with windows which have backing
 * store enabled
 */

static void miBSCheapValidateGC(),  miBSCheapCopyGC(),	miBSCheapDestroyGC();
static void miBSCheapChangeGC ();
static void miBSCheapChangeClip(),  miBSCheapDestroyClip();
static void miBSCheapCopyClip();

static GCFuncs miBSCheapGCFuncs = {
    miBSCheapValidateGC,
    miBSCheapChangeGC,
    miBSCheapCopyGC,
    miBSCheapDestroyGC,
    miBSCheapChangeClip,
    miBSCheapDestroyClip,
    miBSCheapCopyClip,
};

#define CHEAP_FUNC_PROLOGUE(pGC) \
    ((pGC)->funcs = (GCFuncs *) (pGC)->devPrivates[miBSGCIndex].ptr)

#define CHEAP_FUNC_EPILOGUE(pGC) \
    ((pGC)->funcs = &miBSCheapGCFuncs)

/*
 * called from device screen initialization proc.  Gets a GCPrivateIndex
 * and wraps appropriate per-screen functions
 */

miInitializeBackingStore (pScreen, funcs)
    ScreenPtr	pScreen;
    miBSFuncPtr	funcs;
{
    miBSScreenPtr    pScreenPriv;

    pScreenPriv = (miBSScreenPtr) xalloc (sizeof (miBSScreenRec));
    if (!pScreenPriv)
	return;

    if (miBSScreenIndex == -1)
    {
	miBSScreenIndex = AllocateScreenPrivateIndex ();
	if (miBSScreenIndex == -1)
	{
	    xfree ((pointer) pScreenPriv);
	    return;
	}
    }
    if (miBSGCIndex == -1)
	miBSGCIndex = AllocateGCPrivateIndex ();
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pScreenPriv->CreateGC = pScreen->CreateGC;
    pScreenPriv->DestroyWindow = pScreen->DestroyWindow;
    pScreenPriv->funcs = funcs;

    pScreen->CloseScreen = miBSCloseScreen;
    pScreen->GetImage = miBSGetImage;
    pScreen->GetSpans = miBSGetSpans;
    pScreen->ChangeWindowAttributes = miBSChangeWindowAttributes;
    pScreen->CreateGC = miBSCreateGC;
    pScreen->DestroyWindow = miBSDestroyWindow;

    pScreen->SaveDoomedAreas = miBSSaveDoomedAreas;
    pScreen->RestoreAreas = miBSRestoreAreas;
    pScreen->ExposeCopy = miBSExposeCopy;
    pScreen->TranslateBackingStore = miBSTranslateBackingStore;
    pScreen->ClearBackingStore = miBSClearBackingStore;
    pScreen->DrawGuarantee = miBSDrawGuarantee;

    pScreen->devPrivates[miBSScreenIndex].ptr = (pointer) pScreenPriv;
}

/*
 * Screen function wrappers
 */

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((miBSScreenPtr) \
    (pScreen)->devPrivates[miBSScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
miBSCloseScreen (i, pScreen)
    ScreenPtr	pScreen;
{
    miBSScreenPtr   pScreenPriv;

    pScreenPriv = (miBSScreenPtr) pScreen->devPrivates[miBSScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->ChangeWindowAttributes = pScreenPriv->ChangeWindowAttributes;
    pScreen->CreateGC = pScreenPriv->CreateGC;

    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
miBSGetImage (pDrawable, sx, sy, w, h, format, planemask, pdstLine)
    DrawablePtr	    pDrawable;
    int		    sx, sy, w, h;
    unsigned int    format;
    unsigned long   planemask;
    pointer	    pdstLine;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    BoxRec		    bounds;
    WindowPtr		    pWin;
    
    SCREEN_PROLOGUE (pScreen, GetImage);

    if (pDrawable->type == DRAWABLE_WINDOW && ((WindowPtr) pDrawable)->backStorage)
    {
	PixmapPtr	pPixmap;
	miBSWindowPtr	pWindowPriv;
	GCPtr		pGC;

	pWin = (WindowPtr) pDrawable;
	pWindowPriv = (miBSWindowPtr) pWin->backStorage;
	pPixmap = pWindowPriv->pBackingPixmap;

    	bounds.x1 = sx;
    	bounds.y1 = sy;
    	bounds.x2 = bounds.x1 + w;
    	bounds.y2 = bounds.y1 + h;

    	switch ((*pScreen->RectIn) (&pWindowPriv->pSavedRegion, &bounds))
 	{
	case rgnPART:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    pWindowPriv->status = StatusNoPixmap;
	    pGC = GetScratchGC(pPixmap->drawable.depth,
			       pPixmap->drawable.pScreen);
	    if (pGC)
	    {
		ValidateGC ((DrawablePtr) pPixmap, pGC);
		(*pGC->ops->CopyArea)
		    (pDrawable, (DrawablePtr) pPixmap, pGC,
		    bounds.x1, bounds.y1,
		    bounds.x2 - bounds.x1, bounds.y2 - bounds.y1,
		    bounds.x1 + pPixmap->drawable.x - pWin->drawable.x,
		    bounds.y1 + pPixmap->drawable.y - pWin->drawable.y);
		FreeScratchGC(pGC);
	    }
	    pWindowPriv->status = StatusContents;
	    /* fall through */
	case rgnIN:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    (*pScreen->GetImage) ((DrawablePtr) pPixmap,
		sx + pPixmap->drawable.x - pDrawable->x,
 		sy + pPixmap->drawable.y - pDrawable->y,
 		w, h, format, planemask, pdstLine);
	    break;
	case rgnOUT:
	    (*pScreen->GetImage) (pDrawable, sx, sy, w, h,
				  format, planemask, pdstLine);
	    break;
	}
    }
    else
    {
	(*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			      format, planemask, pdstLine);
    }

    SCREEN_EPILOGUE (pScreen, GetImage, miBSGetImage);
}

static unsigned int *
miBSGetSpans (pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr	pDrawable;
    int		wMax;
    DDXPointPtr	ppt;
    int		*pwidth;
    int		nspans;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    unsigned int	    *ret;
    BoxRec		    bounds;
    int			    i;
    WindowPtr		    pWin;
    int			    dx, dy;
    
    SCREEN_PROLOGUE (pScreen, GetSpans);

    if (pDrawable->type == DRAWABLE_WINDOW && ((WindowPtr) pDrawable)->backStorage)
    {
	PixmapPtr	pPixmap;
	miBSWindowPtr	pWindowPriv;
	GCPtr		pGC;

	pWin = (WindowPtr) pDrawable;
	pWindowPriv = (miBSWindowPtr) pWin->backStorage;
	pPixmap = pWindowPriv->pBackingPixmap;

    	bounds.x1 = ppt->x;
    	bounds.y1 = ppt->y;
    	bounds.x2 = bounds.x1 + *pwidth;
    	bounds.y2 = ppt->y;
    	for (i = 0; i < nspans; i++)
    	{
	    if (ppt[i].x < bounds.x1)
	    	bounds.x1 = ppt[i].x;
	    if (ppt[i].x + pwidth[i] > bounds.x2)
	    	bounds.x2 = ppt[i].x + pwidth[i];
	    if (ppt[i].y < bounds.y1)
	    	bounds.y1 = ppt[i].y;
	    else if (ppt[i].y > bounds.y2)
	    	bounds.y2 = ppt[i].y;
    	}
    
    	switch ((*pScreen->RectIn) (&pWindowPriv->pSavedRegion, &bounds))
 	{
	case rgnPART:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    pWindowPriv->status = StatusNoPixmap;
	    pGC = GetScratchGC(pPixmap->drawable.depth,
			       pPixmap->drawable.pScreen);
	    if (pGC)
	    {
		ValidateGC ((DrawablePtr) pPixmap, pGC);
		(*pGC->ops->CopyArea)
		    (pDrawable, (DrawablePtr) pPixmap, pGC,
		    bounds.x1, bounds.y1,
		    bounds.x2 - bounds.x1, bounds.y2 - bounds.y1,
		    bounds.x1 + pPixmap->drawable.x - pWin->drawable.x,
		    bounds.y1 + pPixmap->drawable.y - pWin->drawable.y);
		FreeScratchGC(pGC);
	    }
	    pWindowPriv->status = StatusContents;
	    /* fall through */
	case rgnIN:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    dx = pPixmap->drawable.x - pWin->drawable.x;
	    dy = pPixmap->drawable.y - pWin->drawable.y;
	    for (i = 0; i < nspans; i++)
	    {
		ppt[i].x += dx;
		ppt[i].y += dy;
	    }
	    ret = (*pScreen->GetSpans) ((DrawablePtr) pPixmap,
					wMax, ppt, pwidth, nspans);
	    break;
	case rgnOUT:
	    ret = (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans);
	    break;
	}
    }
    else
    {
	ret = (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans);
    }

    SCREEN_EPILOGUE (pScreen, GetSpans, miBSGetSpans);

    return ret;
}

static Bool
miBSChangeWindowAttributes (pWin, mask)
    WindowPtr	    pWin;
    unsigned long   mask;
{
    ScreenPtr	pScreen;
    Bool	ret;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, ChangeWindowAttributes);

    ret = (*pScreen->ChangeWindowAttributes) (pWin, mask);

    if (ret && (mask & CWBackingStore))
    {
	if (pWin->backingStore != NotUseful || pWin->DIXsaveUnder)
	    miBSAllocate (pWin);
	else
	    miBSFree (pWin);
    }

    SCREEN_EPILOGUE (pScreen, ChangeWindowAttributes, miBSChangeWindowAttributes);

    return ret;
}

/*
 * GC Create wrapper.  Set up the cheap GC func wrappers to track
 * GC validation on BackingStore windows
 */

static Bool
miBSCreateGC (pGC)
    GCPtr   pGC;
{
    ScreenPtr	pScreen = pGC->pScreen;
    Bool	ret;

    SCREEN_PROLOGUE (pScreen, CreateGC);
    
    if (ret = (*pScreen->CreateGC) (pGC))
    {
    	pGC->devPrivates[miBSGCIndex].ptr = (pointer) pGC->funcs;
    	pGC->funcs = &miBSCheapGCFuncs;
    }

    SCREEN_EPILOGUE (pScreen, CreateGC, miBSCreateGC);

    return ret;
}

static Bool
miBSDestroyWindow (pWin)
    WindowPtr	pWin;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    Bool	ret;

    SCREEN_PROLOGUE (pScreen, DestroyWindow);
    
    ret = (*pScreen->DestroyWindow) (pWin);

    miBSFree (pWin);

    SCREEN_EPILOGUE (pScreen, DestroyWindow, miBSDestroyWindow);

    return ret;
}

/*
 * cheap GC func wrappers.  Simply track validation on windows
 * with backing store to enable the real func/op wrappers
 */

static void
miBSCheapValidateGC (pGC, stateChanges, pDrawable)
    GCPtr	    pGC;
    unsigned long   stateChanges;
    DrawablePtr	    pDrawable;
{
    CHEAP_FUNC_PROLOGUE (pGC);
    
    if (pDrawable->type == DRAWABLE_WINDOW &&
        ((WindowPtr) pDrawable)->backStorage != NULL &&
	miBSCreateGCPrivate (pGC))
    {
	(*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);
    }
    else
    {
	(*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);
	CHEAP_FUNC_EPILOGUE (pGC);
    }
}

static void
miBSCheapChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeGC) (pGC, mask);

    CHEAP_FUNC_EPILOGUE (pGC);
}

static void
miBSCheapCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE (pGCDst);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

    CHEAP_FUNC_EPILOGUE (pGCDst);
}

static void
miBSCheapDestroyGC (pGC)
    GCPtr   pGC;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->DestroyGC) (pGC);

    /* leave it unwrapped */
}

static void
miBSCheapChangeClip (pGC, type, pvalue, nrects)
    GCPtr   pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);

    CHEAP_FUNC_EPILOGUE (pGC);
}

static void
miBSCheapCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    CHEAP_FUNC_PROLOGUE (pgcDst);

    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    CHEAP_FUNC_EPILOGUE (pgcDst);
}

static void
miBSCheapDestroyClip(pGC)
    GCPtr	pGC;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (* pGC->funcs->DestroyClip)(pGC);

    CHEAP_FUNC_EPILOGUE (pGC);
}

/*
 * create the full func/op wrappers for a GC
 */

static Bool
miBSCreateGCPrivate (pGC)
    GCPtr   pGC;
{
    miBSGCRec	*pPriv;

    pPriv = (miBSGCRec *) xalloc (sizeof (miBSGCRec));
    if (!pPriv)
	return FALSE;
    pPriv->pBackingGC = NULL;
    pPriv->guarantee = GuaranteeNothing;
    pPriv->serialNumber = 0;
    pPriv->stateChanges = (1 << GCLastBit + 1) - 1;
    pPriv->wrapOps = pGC->ops;
    pPriv->wrapFuncs = pGC->funcs;
    pGC->funcs = &miBSGCFuncs;
    pGC->ops = &miBSGCOps;
    pGC->devPrivates[miBSGCIndex].ptr = (pointer) pPriv;
    return TRUE;
}

static void
miBSDestroyGCPrivate (pGC)
    GCPtr   pGC;
{
    miBSGCRec	*pPriv;

    pPriv = (miBSGCRec *) pGC->devPrivates[miBSGCIndex].ptr;
    if (pPriv)
    {
	pGC->devPrivates[miBSGCIndex].ptr = (pointer) pPriv->wrapFuncs;
	pGC->funcs = &miBSCheapGCFuncs;
	pGC->ops = pPriv->wrapOps;
	if (pPriv->pBackingGC)
	    FreeGC (pPriv->pBackingGC, (GContext) 0);
	xfree ((pointer) pPriv);
    }
}

/*
 * GC ops -- wrap each GC operation with our own function
 */

/*-
 *-----------------------------------------------------------------------
 * miBSFillSpans --
 *	Perform a FillSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    DDXPointPtr	pptCopy;
    int 	  	*pwidthCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nInit*sizeof(DDXPointRec));
    pwidthCopy=(int *)ALLOCATE_LOCAL(nInit*sizeof(int));
    if (pptCopy && pwidthCopy)
    {
	bcopy((char *)pptInit,(char *)pptCopy,nInit*sizeof(DDXPointRec));
	bcopy((char *)pwidthInit,(char *)pwidthCopy,nInit*sizeof(int));

	(* pGC->ops->FillSpans)(pDrawable, pGC, nInit, pptInit,
			     pwidthInit, fSorted);
	(* pBackingGC->ops->FillSpans)(pBackingDrawable,
				  pBackingGC, nInit, pptCopy, pwidthCopy,
				  fSorted);
    }
    if (pwidthCopy) DEALLOCATE_LOCAL(pwidthCopy);
    if (pptCopy) DEALLOCATE_LOCAL(pptCopy);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSSetSpans --
 *	Perform a SetSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    DDXPointPtr	pptCopy;
    int 	*pwidthCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nspans*sizeof(DDXPointRec));
    pwidthCopy=(int *)ALLOCATE_LOCAL(nspans*sizeof(int));
    if (pptCopy && pwidthCopy)
    {
	bcopy((char *)ppt,(char *)pptCopy,nspans*sizeof(DDXPointRec));
	bcopy((char *)pwidth,(char *)pwidthCopy,nspans*sizeof(int));

	(* pGC->ops->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth,
			    nspans, fSorted);
	(* pBackingGC->ops->SetSpans)(pBackingDrawable, pBackingGC,
				 psrc, pptCopy, pwidthCopy, nspans,
				 fSorted);
    }
    if (pwidthCopy) DEALLOCATE_LOCAL(pwidthCopy);
    if (pptCopy) DEALLOCATE_LOCAL(pptCopy);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPutImage --
 *	Perform a PutImage, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int		  depth;
    int	    	  x;
    int	    	  y;
    int	    	  w;
    int	    	  h;
    int	    	  format;
    char    	  *pBits;
{
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    (*pGC->ops->PutImage)(pDrawable, pGC,
		     depth, x, y, w, h, leftPad, format, pBits);
    (*pBackingGC->ops->PutImage)(pBackingDrawable, pBackingGC,
		     depth, x, y, w, h, leftPad, format, pBits);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSDoCopy --
 *	Perform a CopyArea or CopyPlane within a window that has backing
 *	store enabled.
 *
 * Results:
 *	TRUE if the copy was performed or FALSE if a regular one should
 *	be done.
 *
 * Side Effects:
 *	Things are copied (no s***!)
 *
 * Notes:
 *	The idea here is to form two regions that cover the source box.
 *	One contains the exposed rectangles while the other contains
 *	the obscured ones. An array of <box, drawable> pairs is then
 *	formed where the <box> indicates the area to be copied and the
 *	<drawable> indicates from where it is to be copied (exposed regions
 *	come from the screen while obscured ones come from the backing
 *	pixmap). The array 'sequence' is then filled with the indices of
 *	the pairs in the order in which they should be copied to prevent
 *	things from getting screwed up. A call is also made through the
 *	backingGC to take care of any copying into the backing pixmap.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miBSDoCopy(pWin, pGC, srcx, srcy, w, h, dstx, dsty, plane, copyProc, ppRgn)
    WindowPtr	  pWin;	    	    /* Window being scrolled */
    GCPtr   	  pGC;	    	    /* GC we're called through */
    int	    	  srcx;	    	    /* X of source rectangle */
    int	    	  srcy;	    	    /* Y of source rectangle */
    int	    	  w;	    	    /* Width of source rectangle */
    int	    	  h;	    	    /* Height of source rectangle */
    int	    	  dstx;	    	    /* X of destination rectangle */
    int	    	  dsty;	    	    /* Y of destination rectangle */
    unsigned long plane;    	    /* Plane to copy (0 for CopyArea) */
    RegionPtr  	  (*copyProc)();    /* Procedure to call to perform the copy */
    RegionPtr	  *ppRgn;	    /* resultant Graphics Expose region */
{
    RegionPtr 	    	pRgnExp;    /* Exposed region */
    RegionPtr	  	pRgnObs;    /* Obscured region */
    BoxRec	  	box;	    /* Source box (screen coord) */
    struct BoxDraw {
	BoxPtr	  	pBox;	    	/* Source box */
	enum {
	    win, pix
	}   	  	source;	    	/* Place from which to copy */
    }	    	  	*boxes;	    /* Array of box/drawable pairs covering
				     * source box. */
    int  	  	*sequence;  /* Sequence of boxes to move */
    register int  	i, j, k, l, y;
    register BoxPtr	pBox;
    int	    	  	dx, dy, nrects;
    Bool    	  	graphicsExposures;
    RegionPtr	  	(*pixCopyProc)();
    int			numRectsExp, numRectsObs;
    BoxPtr		pBoxExp, pBoxObs;

    SETUP_BACKING_VERBOSE (pWin, pGC);

    /*
     * Create a region of exposed boxes in pRgnExp.
     */
    box.x1 = srcx + pWin->drawable.x;
    box.x2 = box.x1 + w;
    box.y1 = srcy + pWin->drawable.y;
    box.y2 = box.y1 + h;
    
    pRgnExp = (*pGC->pScreen->RegionCreate) (&box, 1);
    (*pGC->pScreen->Intersect) (pRgnExp, pRgnExp, &pWin->clipList);
    pRgnObs = (*pGC->pScreen->RegionCreate) (NULL, 1);
    (* pGC->pScreen->Inverse) (pRgnObs, pRgnExp, &box);

    /*
     * Translate regions into window coordinates for proper calls
     * to the copyProc, then make sure none of the obscured region sticks
     * into invalid areas of the backing pixmap.
     */
    (*pGC->pScreen->TranslateRegion) (pRgnExp,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
    (*pGC->pScreen->TranslateRegion) (pRgnObs,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
    (*pGC->pScreen->Intersect)(pRgnObs, pRgnObs, &pBackingStore->pSavedRegion);

    /*
     * If the obscured region is empty, there's no point being fancy.
     */
    if (!(*pGC->pScreen->RegionNotEmpty) (pRgnObs))
    {
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
	(*pGC->pScreen->RegionDestroy) (pRgnObs);

	return (FALSE);
    }

    numRectsExp = REGION_NUM_RECTS(pRgnExp);
    pBoxExp = REGION_RECTS(pRgnExp);
    pBoxObs = REGION_RECTS(pRgnObs);
    numRectsObs = REGION_NUM_RECTS(pRgnObs);
    nrects = numRectsExp + numRectsObs;
    
    boxes = (struct BoxDraw *)ALLOCATE_LOCAL(nrects * sizeof(struct BoxDraw));
    sequence = (int *) ALLOCATE_LOCAL(nrects * sizeof(int));
    *ppRgn = NULL;

    if (!boxes || !sequence)
    {
	if (sequence) DEALLOCATE_LOCAL(sequence);
	if (boxes) DEALLOCATE_LOCAL(boxes);
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
	(*pGC->pScreen->RegionDestroy) (pRgnObs);

	return(TRUE);
    }

    /*
     * Order the boxes in the two regions so we know from which drawable
     * to copy which box, storing the result in the boxes array
     */
    for (i = 0, j = 0, k = 0;
	 (i < numRectsExp) && (j < numRectsObs);
	 k++)
    {
	if (pBoxExp[i].y1 < pBoxObs[j].y1)
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	}
	else if ((pBoxObs[j].y1 < pBoxExp[i].y1) ||
		 (pBoxObs[j].x1 < pBoxExp[i].x1))
	{
	    boxes[k].pBox = &pBoxObs[j];
	    boxes[k].source = pix;
	    j++;
	}
	else
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	}
    }

    /*
     * Catch any leftover boxes from either region (note that only
     * one can have leftover boxes...)
     */
    if (i != numRectsExp)
    {
	do
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	    k++;
	} while (i < numRectsExp);

    }
    else
    {
	do
	{
	    boxes[k].pBox = &pBoxObs[j];
	    boxes[k].source = pix;
	    j++;
	    k++;
	} while (j < numRectsObs);
    }
    
    if (dsty <= srcy)
    {
	/*
	 * Scroll up or vertically stationary, so vertical order is ok.
	 */
	if (dstx <= srcx)
	{
	    /*
	     * Scroll left or horizontally stationary, so horizontal order
	     * is ok as well.
	     */
	    for (i = 0; i < nrects; i++)
	    {
		sequence[i] = i;
	    }
	}
	else
	{
	    /*
	     * Scroll right. Need to reverse the rectangles within each
	     * band.
	     */
	    for (i = 0, j = 1, k = 0;
		 i < nrects;
		 j = i + 1, k = i)
	    {
		y = boxes[i].pBox->y1;
		while ((j < nrects) && (boxes[j].pBox->y1 == y))
		{
		    j++;
		}
		for (j--; j >= k; j--, i++)
		{
		    sequence[i] = j;
		}
	    }
	}
    }
    else
    {
	/*
	 * Scroll down. Must reverse vertical banding, at least.
	 */
	if (dstx < srcx)
	{
	    /*
	     * Scroll left. Horizontal order is ok.
	     */
	    for (i = nrects - 1, j = i - 1, k = i, l = 0;
		 i >= 0;
		 j = i - 1, k = i)
	    {
		/*
		 * Find extent of current horizontal band, then reverse
		 * the order of the whole band.
		 */
		y = boxes[i].pBox->y1;
		while ((j >= 0) && (boxes[j].pBox->y1 == y))
		{
		    j--;
		}
		for (j++; j <= k; j++, i--, l++)
		{
		    sequence[l] = j;
		}
	    }
	}
	else
	{
	    /*
	     * Scroll right or horizontal stationary.
	     * Reverse horizontal order as well (if stationary, horizontal
	     * order can be swapped without penalty and this is faster
             * to compute).
	     */
	    for (i = 0, j = nrects - 1; i < nrects; i++, j--)
	    {
		sequence[i] = j;
	    }
	}
    }
	    
    /*
     * XXX: To avoid getting multiple NoExpose events from this operation,
     * we turn OFF graphicsExposures in the gc and deal with any uncopied
     * areas later, if there's something not in backing-store.
     */

    graphicsExposures = pGC->graphicsExposures;
    pGC->graphicsExposures = FALSE;
    
    dx = dstx - srcx;
    dy = dsty - srcy;

    /*
     * Figure out which copy procedure to use from the backing GC. Note we
     * must do this because some implementations (sun's, e.g.) have
     * pBackingGC a fake GC with the real one below it, thus the devPriv for
     * pBackingGC won't be what the output library expects.
     */
    if (plane != 0)
    {
	pixCopyProc = pBackingGC->ops->CopyPlane;
    }
    else
    {
	pixCopyProc = pBackingGC->ops->CopyArea;
    }
    
    for (i = 0; i < nrects; i++)
    {
	pBox = boxes[sequence[i]].pBox;
	
	/*
	 * If we're copying from the pixmap, we need to place its contents
	 * onto the screen before scrolling the pixmap itself. If we're copying
	 * from the window, we need to copy its contents into the pixmap before
	 * we scroll the window itself.
	 */
	if (boxes[sequence[i]].source == pix)
	{
	    (void) (* copyProc) (pBackingDrawable, pWin, pGC,
			  pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	    (void) (* pixCopyProc) (pBackingDrawable, pBackingDrawable, pBackingGC,
			     pBox->x1, pBox->y1,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx, pBox->y1 + dy, plane);
	}
	else
	{
	    (void) (* pixCopyProc) (pWin, pBackingDrawable, pBackingGC,
			     pBox->x1, pBox->y1,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx, pBox->y1 + dy, plane);
	    (void) (* copyProc) (pWin, pWin, pGC,
			  pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
    }
    DEALLOCATE_LOCAL(sequence);
    DEALLOCATE_LOCAL(boxes);

    pGC->graphicsExposures = graphicsExposures;
    if (graphicsExposures)
    {
	/*
	 * Form union of rgnExp and rgnObs and see if covers entire area
	 * to be copied.  Store the resultant region for miBSCopyArea
	 * to return to dispatch which will send the appropriate expose
	 * events.
	 */
	(* pGC->pScreen->Union) (pRgnExp, pRgnExp, pRgnObs);
	box.x1 = srcx;
	box.x2 = srcx + w;
	box.y1 = srcy;
	box.y2 = srcy + h;
	if ((* pGC->pScreen->RectIn) (pRgnExp, &box) == rgnIN)
	    (*pGC->pScreen->RegionEmpty) (pRgnExp);
	else
	    (* pGC->pScreen->Inverse) (pRgnExp, pRgnExp, &box);
	*ppRgn = pRgnExp;
    }
    else
    {
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
    }
    (*pGC->pScreen->RegionDestroy) (pRgnObs);

    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyArea --
 *	Perform a CopyArea from the source to the destination, extracting
 *	from the source's backing-store and storing into the destination's
 *	backing-store without messing anything up. If the source and
 *	destination are different, there's not too much to worry about:
 *	we can just issue several calls to the regular CopyArea function.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	pixExposed = 0, winExposed = 0;

    SETUP_BACKING(pDst, pGC);

    PROLOGUE(pGC);

    if ((pSrc != pDst) ||
	(!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
		     (unsigned long) 0, pGC->ops->CopyArea, &winExposed)))
    {
	/*
	 * always copy to the backing store first, miBSDoCopy
	 * returns FALSE if the *source* region is disjoint
	 * from the backing store saved region.  So, copying
	 * *to* the backing store is always safe
	 */
	if (pGC->clientClipType != CT_PIXMAP)
	{
	    /*
	     * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
	     * the backing store.  An unnecessary optimisation,
	     * but a useful one when GetSpans is slow.
	     */
	    pExtents = (*pDst->pScreen->RegionExtents)
		    (pBackingGC->clientClip);
	    bsrcx = srcx;
	    bsrcy = srcy;
	    bw = w;
	    bh = h;
	    bdstx = dstx;
	    bdsty = dsty;
	    dx = pExtents->x1 - bdstx;
	    if (dx > 0)
	    {
		bsrcx += dx;
		bdstx += dx;
		bw -= dx;
	    }
	    dy = pExtents->y1 - bdsty;
	    if (dy > 0)
	    {
		bsrcy += dy;
		bdsty += dy;
		bh -= dy;
	    }
	    dx = (bdstx + bw) - pExtents->x2;
	    if (dx > 0)
		bw -= dx;
	    dy = (bdsty + bh) - pExtents->y2;
	    if (dy > 0)
		bh -= dy;
	    if (bw > 0 && bh > 0)
		pixExposed = (* pBackingGC->ops->CopyArea) (pSrc, 
			    pBackingDrawable, pBackingGC, 
			    bsrcx, bsrcy, bw, bh, bdstx, bdsty);
	}
	else
	    pixExposed = (* pBackingGC->ops->CopyArea) (pSrc, 
			    pBackingDrawable, pBackingGC,
			    srcx, srcy, w, h, dstx, dsty);

	winExposed = (* pGC->ops->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
    }

    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed){
	    (*pDst->pScreen->Union) (winExposed, winExposed, pixExposed);
	    (*pDst->pScreen->RegionDestroy) (pixExposed);
	}
    } else
	winExposed = pixExposed;

    EPILOGUE (pGC);

    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyPlane --
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    register GC   *pGC;
    int     	  srcx,
		  srcy;
    int     	  w,
		  h;
    int     	  dstx,
		  dsty;
    unsigned long  plane;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	winExposed = 0, pixExposed = 0;
    SETUP_BACKING(pDst, pGC);

    PROLOGUE(pGC);

    if ((pSrc != pDst) ||
	(!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
		     plane,  pGC->ops->CopyPlane, &winExposed)))
    {
	/*
	 * always copy to the backing store first, miBSDoCopy
	 * returns FALSE if the *source* region is disjoint
	 * from the backing store saved region.  So, copying
	 * *to* the backing store is always safe
	 */
	if (pGC->clientClipType != CT_PIXMAP)
	{
	    /*
	     * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
	     * the backing store.  An unnecessary optimisation,
	     * but a useful one when GetSpans is slow.
	     */
	    pExtents = (*pDst->pScreen->RegionExtents) (pBackingGC->clientClip);
	    bsrcx = srcx;
	    bsrcy = srcy;
	    bw = w;
	    bh = h;
	    bdstx = dstx;
	    bdsty = dsty;
	    dx = pExtents->x1 - bdstx;
	    if (dx > 0)
	    {
		bsrcx += dx;
		bdstx += dx;
		bw -= dx;
	    }
	    dy = pExtents->y1 - bdsty;
	    if (dy > 0)
	    {
		bsrcy += dy;
		bdsty += dy;
		bh -= dy;
	    }
	    dx = (bdstx + bw) - pExtents->x2;
	    if (dx > 0)
		bw -= dx;
	    dy = (bdsty + bh) - pExtents->y2;
	    if (dy > 0)
		bh -= dy;
	    if (bw > 0 && bh > 0)
		pixExposed = (* pBackingGC->ops->CopyPlane) (pSrc, 
				    pBackingDrawable,
				    pBackingGC, bsrcx, bsrcy, bw, bh,
				    bdstx, bdsty, plane);
	}
	else
	    pixExposed = (* pBackingGC->ops->CopyPlane) (pSrc, 
				    pBackingDrawable,
				    pBackingGC, srcx, srcy, w, h,
				    dstx, dsty, plane);

	winExposed = (* pGC->ops->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
			      dstx, dsty, plane);
	
    }

    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed)
	{
	    (*pDst->pScreen->Union) (winExposed, winExposed, pixExposed);
	    (*pDst->pScreen->RegionDestroy) (pixExposed);
	}
    } else
	winExposed = pixExposed;

    EPILOGUE (pGC);

    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyPoint --
 *	Perform a PolyPoint, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    xPoint	  *pptCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (xPoint *)ALLOCATE_LOCAL(npt*sizeof(xPoint));
    if (pptCopy)
    {
	bcopy((char *)pptInit,(char *)pptCopy,npt*sizeof(xPoint));

	(* pGC->ops->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);

	(* pBackingGC->ops->PolyPoint) (pBackingDrawable,
				   pBackingGC, mode, npt, pptCopy);

	DEALLOCATE_LOCAL(pptCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyLines --
 *	Perform a Polylines, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    DDXPointPtr	pptCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(npt*sizeof(DDXPointRec));
    if (pptCopy)
    {
	bcopy((char *)pptInit,(char *)pptCopy,npt*sizeof(DDXPointRec));

	(* pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit);
	(* pBackingGC->ops->Polylines)(pBackingDrawable,
				  pBackingGC, mode, npt, pptCopy);
	DEALLOCATE_LOCAL(pptCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolySegment --
 *	Perform a PolySegment, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolySegment(pDrawable, pGC, nseg, pSegs)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    xSegment	*pSegsCopy;

    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pSegsCopy = (xSegment *)ALLOCATE_LOCAL(nseg*sizeof(xSegment));
    if (pSegsCopy)
    {
	bcopy((char *)pSegs,(char *)pSegsCopy,nseg*sizeof(xSegment));

	(* pGC->ops->PolySegment)(pDrawable, pGC, nseg, pSegs);
	(* pBackingGC->ops->PolySegment)(pBackingDrawable,
				    pBackingGC, nseg, pSegsCopy);

	DEALLOCATE_LOCAL(pSegsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyRectangle --
 *	Perform a PolyRectangle, routing output to backing-store as needed.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyRectangle(pDrawable, pGC, nrects, pRects)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    xRectangle	*pRectsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pRectsCopy =(xRectangle *)ALLOCATE_LOCAL(nrects*sizeof(xRectangle));
    if (pRectsCopy)
    {
	bcopy((char *)pRects,(char *)pRectsCopy,nrects*sizeof(xRectangle));

	(* pGC->ops->PolyRectangle)(pDrawable, pGC, nrects, pRects);
	(* pBackingGC->ops->PolyRectangle)(pBackingDrawable,
				      pBackingGC, nrects, pRectsCopy);

	DEALLOCATE_LOCAL(pRectsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyArc --
 *	Perform a PolyArc, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc  *pArcsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
    if (pArcsCopy)
    {
	bcopy((char *)parcs,(char *)pArcsCopy,narcs*sizeof(xArc));

	(* pGC->ops->PolyArc)(pDrawable, pGC, narcs, parcs);
	(* pBackingGC->ops->PolyArc)(pBackingDrawable, pBackingGC,
				narcs, pArcsCopy);

	DEALLOCATE_LOCAL(pArcsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSFillPolygon --
 *	Perform a FillPolygon, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFillPolygon(pDrawable, pGC, shape, mode, count, pPts)
    DrawablePtr		pDrawable;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    DDXPointPtr	pPtsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pPtsCopy = (DDXPointPtr)ALLOCATE_LOCAL(count*sizeof(DDXPointRec));
    if (pPtsCopy)
    {
	bcopy((char *)pPts,(char *)pPtsCopy,count*sizeof(DDXPointRec));
	(* pGC->ops->FillPolygon)(pDrawable, pGC, shape, mode, count, pPts);
	(* pBackingGC->ops->FillPolygon)(pBackingDrawable,
				    pBackingGC, shape, mode,
				    count, pPtsCopy);

	DEALLOCATE_LOCAL(pPtsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillRect --
 *	Perform a PolyFillRect, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	*pRectCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pRectCopy =
	(xRectangle *)ALLOCATE_LOCAL(nrectFill*sizeof(xRectangle));
    if (pRectCopy)
    {
	bcopy((char *)prectInit,(char *)pRectCopy,
	      nrectFill*sizeof(xRectangle));

	(* pGC->ops->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
	(* pBackingGC->ops->PolyFillRect)(pBackingDrawable,
				     pBackingGC, nrectFill, pRectCopy);

	DEALLOCATE_LOCAL(pRectCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillArc --
 *	Perform a PolyFillArc, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyFillArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc  *pArcsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
    if (pArcsCopy)
    {
	bcopy((char *)parcs,(char *)pArcsCopy,narcs*sizeof(xArc));
	(* pGC->ops->PolyFillArc)(pDrawable, pGC, narcs, parcs);
	(* pBackingGC->ops->PolyFillArc)(pBackingDrawable,
				    pBackingGC, narcs, pArcsCopy);
	DEALLOCATE_LOCAL(pArcsCopy);
    }

    EPILOGUE (pGC);
}


/*-
 *-----------------------------------------------------------------------
 * miBSPolyText8 --
 *	Perform a PolyText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static int
miBSPolyText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    int	    result;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    result = (* pGC->ops->PolyText8)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->PolyText8)(pBackingDrawable,
			      pBackingGC, x, y, count, chars);

    EPILOGUE (pGC);
    return result;
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyText16 --
 *	Perform a PolyText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static int
miBSPolyText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    int	result;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    result = (* pGC->ops->PolyText16)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->PolyText16)(pBackingDrawable,
			       pBackingGC, x, y, count, chars);

    EPILOGUE (pGC);

    return result;
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText8 --
 *	Perform a ImageText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->ImageText8)(pBackingDrawable,
			       pBackingGC, x, y, count, chars);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText16 --
 *	Perform a ImageText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageText16)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->ImageText16)(pBackingDrawable,
				pBackingGC, x, y, count, chars);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageGlyphBlt --
 *	Perform a ImageGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
			     pglyphBase);
    (* pBackingGC->ops->ImageGlyphBlt)(pBackingDrawable,
				  pBackingGC, x, y, nglyph, ppci,
				  pglyphBase);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyGlyphBlt --
 *	Perform a PolyGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph,
			    ppci, pglyphBase);
    (* pBackingGC->ops->PolyGlyphBlt)(pBackingDrawable,
				 pBackingGC, x, y, nglyph, ppci,
				 pglyphBase);
    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPushPixels --
 *	Perform a PushPixels, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
    SETUP_BACKING (pDst, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->PushPixels)(pGC, pBitMap, pDst, w, h, x, y);
    (* pBackingGC->ops->PushPixels)(pBackingGC, pBitMap,
			       pBackingDrawable, w, h,
			       x, y);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSLineHelper --
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSLineHelper(pDrawable, pGC, cap, npts, pts, xOrg, yOrg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    Bool	cap;
    int		npts;
    pointer	pts;
    int		xOrg, yOrg;
{
    SETUP_BACKING_TERSE (pGC);
    PROLOGUE(pGC);

    (* pGC->ops->LineHelper)(pDrawable, pGC, cap, npts, pts, xOrg, yOrg);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSClearBackingStore --
 *	Clear the given area of the backing pixmap with the background of
 *	the window, whatever it is. If generateExposures is TRUE, generate
 *	exposure events for the area. Note that if the area has any
 *	part outside the saved portions of the window, we do not allow the
 *	count in the expose events to be 0, since there will be more
 *	expose events to come.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Areas of pixmap are cleared and Expose events are generated.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSClearBackingStore(pWin, x, y, w, h, generateExposures)
    WindowPtr	  	pWin;
    int	    	  	x;
    int	    	  	y;
    int	    	  	w;
    int	    	  	h;
    Bool    	  	generateExposures;
{
    RegionPtr	  	pRgn;
    int	    	  	i;
    miBSWindowPtr	pBackingStore;
    ScreenPtr	  	pScreen;
    GCPtr   	  	pGC;
    int	    	  	ts_x_origin,
			ts_y_origin;
    XID	    	  	gcvalues[4];
    unsigned long 	gcmask;
    xRectangle	  	*rects;
    BoxPtr  	  	pBox;
    BoxRec  	  	box;
    PixUnion		background;
    char		backgroundState;
    int			numRects;

    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pScreen = pWin->drawable.pScreen;

    if (pBackingStore->status == StatusNoPixmap)
	return;
    
    if (w == 0)
	w = (int) pWin->drawable.width - x;
    if (h == 0)
	h = (int) pWin->drawable.height - y;

    box.x1 = x;
    box.y1 = y;
    box.x2 = x + w;
    box.y2 = y + h;
    pRgn = (*pWin->drawable.pScreen->RegionCreate)(&box, 1);
    if (!pRgn)
	return;
    (* pScreen->Intersect) (pRgn, pRgn, &pBackingStore->pSavedRegion);

    if ((* pScreen->RegionNotEmpty) (pRgn))
    {
	/*
	 * if clearing entire window, simply make new virtual
	 * tile.  For the root window, we also destroy the pixmap
	 * to save a pile of memory
	 */
	if (x == 0 && y == 0 &&
 	    w == pWin->drawable.width &&
 	    h == pWin->drawable.height)
	{
	    if (!pWin->parent)
		miDestroyBSPixmap (pWin);
	    if (pBackingStore->status != StatusContents)
		 miTileVirtualBS (pWin);
	}

	ts_x_origin = ts_y_origin = 0;

	backgroundState = pWin->backgroundState;
	background = pWin->background;
	if (backgroundState == ParentRelative) {
	    WindowPtr	pParent;

	    pParent = pWin;
	    while (pParent->backgroundState == ParentRelative) {
		ts_x_origin -= pParent->origin.x;
		ts_y_origin -= pParent->origin.y;
		pParent = pParent->parent;
	    }
	    backgroundState = pParent->backgroundState;
	    background = pParent->background;
	}

	if ((backgroundState != None) &&
	    ((pBackingStore->status == StatusContents) ||
	     !SameBackground (pBackingStore->backgroundState,
			      pBackingStore->background,
			      backgroundState,
			      background)))
	{
	    if (!pBackingStore->pBackingPixmap)
		miCreateBSPixmap(pWin);

	    pGC = GetScratchGC(pWin->drawable.depth, pScreen);
	    if (pGC && pBackingStore->pBackingPixmap)
	    {
		/*
		 * First take care of any ParentRelative stuff by altering the
		 * tile/stipple origin to match the coordinates of the upper-left
		 * corner of the first ancestor without a ParentRelative background.
		 * This coordinate is, of course, negative.
		 */
	    
		if (backgroundState == BackgroundPixel)
		{
		    gcvalues[0] = (XID) background.pixel;
		    gcvalues[1] = FillSolid;
		    gcmask = GCForeground|GCFillStyle;
		}
		else
		{
		    gcvalues[0] = FillTiled;
		    gcvalues[1] = (XID) background.pixmap;
		    gcmask = GCFillStyle|GCTile;
		}
		gcvalues[2] = ts_x_origin;
		gcvalues[3] = ts_y_origin;
		gcmask |= GCTileStipXOrigin|GCTileStipYOrigin;
		DoChangeGC(pGC, gcmask, gcvalues, TRUE);
		ValidateGC((DrawablePtr)pBackingStore->pBackingPixmap, pGC);
    
		/*
		 * Figure out the array of rectangles to fill and fill them with
		 * PolyFillRect in the proper mode, as set in the GC above.
		 */
		numRects = REGION_NUM_RECTS(pRgn);
		rects = (xRectangle *)ALLOCATE_LOCAL(numRects*sizeof(xRectangle));
	    
		if (rects)
		{
		    for (i = 0, pBox = REGION_RECTS(pRgn);
			 i < numRects;
			 i++, pBox++)
		    {
			rects[i].x = pBox->x1;
			rects[i].y = pBox->y1;
			rects[i].width = pBox->x2 - pBox->x1;
			rects[i].height = pBox->y2 - pBox->y1;
		    }
		    (* pGC->ops->PolyFillRect) (pBackingStore->pBackingPixmap,
				       pGC, numRects, rects);
		    DEALLOCATE_LOCAL(rects);
		}	
		FreeScratchGC(pGC);
	    }
	}	

	if (generateExposures) {
	    int	  offset;
	    xEvent *events, *ev;

	    /*
	     * If there are exposed areas in the box the client wanted cleared,
	     * make sure u.expose.count doesn't go to 0. Note that the count
	     * is only a hint, the only guarantee being the last Expose event
	     * from a single operation has a count of 0, so this numbering
	     * scheme is ok.
	     */
	    offset = (((* pScreen->RectIn) (pRgn, &box) != rgnIN)?1:0);

	    numRects = REGION_NUM_RECTS(pRgn);
	    events = (xEvent *)ALLOCATE_LOCAL(numRects*sizeof(xEvent));
	    if (events)
	    {
		for (i = numRects, pBox = REGION_RECTS(pRgn), ev = events;
		     --i >= 0;
		     pBox++, ev++)
		{
		    ev->u.u.type = Expose;
		    ev->u.expose.window = pWin->drawable.id;
		    ev->u.expose.x = pBox->x1;
		    ev->u.expose.y = pBox->y1;
		    ev->u.expose.width = pBox->x2 - pBox->x1;
		    ev->u.expose.height = pBox->y2 - pBox->y1;
		    ev->u.expose.count = i + offset;
		}
		DeliverEvents(pWin, events, numRects, NullWindow);
		DEALLOCATE_LOCAL(events);
	    }
	}
    }

    (* pScreen->RegionDestroy) (pRgn);
}

#ifdef NOTDEF	/* XXX current code ignores this stuff */

/*-
 * miGetImageWithBS
 *
 * use miGetImage and miBSGetImage to form a composite image for a
 * backing stored window
 */

void
miGetImageWithBS ( pDraw, x, y, w, h, format, planemask, pImage)
    DrawablePtr		pDraw;
    int			x, y, w, h;
    unsigned int	format;
    unsigned long	planemask;
    unsigned char	*pImage;
{
    BackingStorePt  *backStorage = NULL;

    /*
     * miGetImage uses the pScreen->GetSpans but GetSpans has been mangled to
     * get bits from backing store for, which we desparately want to avoid at
     * this point as GetImage is expected to return screen contents.
     */
    if (pDraw->type == DRAWABLE_WINDOW)
    {
	backStorage = ((WindowPtr)pDraw)->backStorage;
	((WindowPtr)pDraw)->backStorage = NULL;
    }
    miGetImage( pDraw, x, y, w, h, format, planemask, pImage);
    if (backStorage != NULL)
    {
	((WindowPtr)pDraw)->backStorage = backStorage;
	miBSGetImage((WindowPtr) pDraw, NullPixmap, x, y, w, h,
		     format, planemask, pImage);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSGetImage --
 *	Retrieve the missing pieces of the given image from the window's
 *	backing-store and place them into the given pixmap at the proper
 *	location.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Parts of pImage are modified.
 *
 *-----------------------------------------------------------------------
 */

void
miBSGetImage (pWin, pOldPixmapPtr, x, y, w, h, format, planeMask, pImage)
    WindowPtr	  pWin;	    	    /* Source window with bstore */
    PixmapPtr	  pOldPixmapPtr;    /* pixmap containing screen contents */
    int	    	  x;	    	    /* Window-relative x of source box */
    int	    	  y;	    	    /* Window-relative y of source box */
    int	    	  w;	    	    /* Width of source box */
    int	    	  h;	    	    /* Height of source box */
    unsigned int  format;   	    /* Format for request */
    unsigned long  planeMask;	    /* Mask of planes to fetch */
    pointer	  pImage;	    /* pointer to space allocated by caller */
{
    ScreenPtr	  	pScreen;    /* Screen for proc vectors */
    PixmapPtr		pNewPixmapPtr; /* local pixmap hack */
    PixmapPtr		pPixmapPtr;
    RegionPtr		pRgn;
    BoxRec		box;
    GCPtr		pGC;
    static void		miBSDoGetImage ();
    int			depth;

    pScreen = pWin->drawable.pScreen;

    /*
     * First figure out what part of the source box is actually not visible
     * by taking the inverse of the not-clipped-by-children region.
     */

    pRgn = (* pScreen->RegionCreate)(NULL, 1);
    box.x1 = x + pWin->absCorner.x;
    box.y1 = y + pWin->absCorner.y;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;
    
    (* pScreen->Inverse)(pRgn, &pWin->borderClip, &box);

    /*
     * Nothing that wasn't visible -- return immediately.
     */
    if (!(* pScreen->RegionNotEmpty) (pRgn))
    {
	(* pScreen->RegionDestroy) (pRgn);
	return;
    }

    /*
     * if no pixmap was given to us, create one now
     */

    depth = format == ZPixmap ? pWin->drawable.depth : 1;
    
    pGC = GetScratchGC (depth, pScreen);
    if (!pGC)
    {
	(* pScreen->RegionDestroy) (pRgn);
	return;
    }

    /*
     * make sure the CopyArea operations below never
     * end up sending NoExpose or GraphicsExpose events.
     */
    pGC->graphicsExposures = FALSE;
    if (!pOldPixmapPtr && pImage) {
	pNewPixmapPtr = (*pScreen->CreatePixmap) (pScreen, w, h, depth);
	if (pNewPixmapPtr) {
	    ValidateGC ((DrawablePtr)pNewPixmapPtr, pGC);
	    (*pGC->ops->PutImage) (pNewPixmapPtr, pGC,
 			      depth, 0, 0, w, h, 0,
			      ZPixmap, pImage);
	}
	pPixmapPtr = pNewPixmapPtr;
    }
    else
    {
	pNewPixmapPtr = NullPixmap;
	pPixmapPtr = pOldPixmapPtr;
	if (pPixmapPtr)
	    ValidateGC ((DrawablePtr)pPixmapPtr, pGC);
    }

    /*
     * translate to window-relative coordinates
     */

    if (pPixmapPtr)
    {
	(* pScreen->TranslateRegion) (pRgn, -pWin->absCorner.x, -pWin->absCorner.y);

	miBSDoGetImage (pWin, pPixmapPtr, pRgn, x, y, pGC, planeMask);
    }

    /*
     * now we have a composite view in pNewPixmapPtr; create the resultant
     * image
     */

    if (pNewPixmapPtr)
    {
	(*pScreen->GetImage) (pNewPixmapPtr, 0, 0, w, h,
 			      format, (format == ZPixmap) ? planeMask : 1, pImage);
	(*pScreen->DestroyPixmap) (pNewPixmapPtr);
    }
    FreeScratchGC (pGC);
    (*pScreen->RegionDestroy) (pRgn);
}

#endif

/*
 * fill a region of the destination with virtual bits
 *
 * pRgn is offset by (x, y) into the drawable
 */

static void
miBSFillVirtualBits (pDrawable, pGC, pRgn, x, y, state, pixunion, planeMask)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    RegionPtr		pRgn;
    int			x, y;
    int			state;
    PixUnion		pixunion;
    unsigned long	planeMask;
{
    int		i;
    BITS32	gcmask;
    XID		gcval[5];
    xRectangle	*pRect;
    BoxPtr	pBox;
    WindowPtr	pWin;
    int		numRects;

    if (state == None)
	return;
    numRects = REGION_NUM_RECTS(pRgn);
    pRect = (xRectangle *)ALLOCATE_LOCAL(numRects * sizeof(xRectangle));
    if (!pRect)
	return;
    pWin = 0;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDrawable;
	if (!pWin->backStorage)
	    pWin = 0;
    }
    i = 0;
    gcmask = 0;
    gcval[i++] = planeMask;
    gcmask |= GCPlaneMask;
    if (state == BackgroundPixel)
    {
	if (pGC->fgPixel != pixunion.pixel)
	{
	    gcval[i++] = (XID) pixunion.pixel;
	    gcmask |= GCForeground;
	}
	if (pGC->fillStyle != FillSolid)
	{
	    gcval[i++] = (XID) FillSolid;
	    gcmask |= GCFillStyle;
	}
    }
    else
    {
	if (pGC->fillStyle != FillTiled)
	{
	    gcval[i++] = (XID) FillTiled;
	    gcmask |= GCFillStyle;
	}
	if (pGC->tileIsPixel || pGC->tile.pixmap != pixunion.pixmap)
	{
	    gcval[i++] = (XID) pixunion.pixmap;
	    gcmask |= GCTile;
	}
	if (pGC->patOrg.x != -x)
	{
	    gcval[i++] = (XID) -x;
	    gcmask |= GCTileStipXOrigin;
	}
	if (pGC->patOrg.y != -y)
	{
	    gcval[i++] = (XID) -y;
	    gcmask |= GCTileStipYOrigin;
	}
    }
    if (gcmask)
	DoChangeGC (pGC, gcmask, gcval, 1);

    if (pWin)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    if (pDrawable->serialNumber != pGC->serialNumber)
	ValidateGC (pDrawable, pGC);

    pBox = REGION_RECTS(pRgn);
    for (i = numRects; --i >= 0; pBox++, pRect++)
    {
    	pRect->x = pBox->x1 - x;
	pRect->y = pBox->y1 - y;
	pRect->width = pBox->x2 - pBox->x1;
	pRect->height = pBox->y2 - pBox->y1;
    }
    pRect -= numRects;
    (*pGC->ops->PolyFillRect) (pDrawable, pGC, numRects, pRect);
    if (pWin)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeNothing);
    DEALLOCATE_LOCAL (pRect);
}

#ifdef NOTDEF	/* XXX current code ignores this */
/*
 * copy this window's backing store and all of it's childrens
 * backing store into pPixmap
 *
 * (x, y) is the offset of the pixmap into the window
 */

static void
miBSDoGetImage (pWin, pPixmap, pRgn, x, y, pGC, planeMask)
    WindowPtr	pWin;
    PixmapPtr	pPixmap;
    RegionPtr	pRgn;
    int		x, y;
    GCPtr	pGC;
    unsigned long	planeMask;
{
    miBSWindowPtr	pBackingStore;
    BoxPtr	pBox;
    WindowPtr	pChild;
    RegionPtr	pBackRgn;
    ScreenPtr	pScreen;
    int		dx, dy;
    int		n;
    
    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pBackRgn = (*pScreen->RegionCreate) (NULL, 1);
    if (pWin->backStorage && pBackingStore->status != StatusNoPixmap)
    {
	(*pScreen->Intersect) (pBackRgn, pRgn, &pBackingStore->pSavedRegion);

	if ((*pScreen->RegionNotEmpty) (pBackRgn))
	{
	    if (!pBackingStore->pBackingPixmap &&
 		pBackingStore->backgroundState != ParentRelative &&
		pWin->drawable.depth == pPixmap->drawable.depth)
	    {
		miBSFillVirtualBits ((DrawablePtr)pPixmap, pGC, pBackRgn, x, y,
	    			     pBackingStore->backgroundPixel,
				     pBackingStore->backgroundTile,
				     (PixmapPtr) USE_BACKGROUND_PIXEL,
				     planeMask);
	    }
	    else
	    {
		if (!pBackingStore->pBackingPixmap)
		    miCreateBSPixmap (pWin);

		if (pBackingStore->pBackingPixmap)
		{
		    if (pWin->drawable.depth != pPixmap->drawable.depth &&
			!REGION_NIL(pBackRgn))
		    {
			XID	gcval[3];

			gcval[0] = 1;	/* plane mask */
			gcval[1] = 1;	/* foreground */
			gcval[2] = 0;	/* background */
			DoChangeGC (pGC, GCPlaneMask|GCForeground|GCBackground, gcval, 1);
			ValidateGC ((DrawablePtr) pPixmap, pGC);
		    }
		    pBox = REGION_RECTS(pBackRgn);
		    for (n = REGION_NUM_RECTS(pBackRgn); --n >= 0;)
		    {
			if (pWin->drawable.depth == pPixmap->drawable.depth)
			    (*pGC->ops->CopyArea) (pBackingStore->pBackingPixmap,
 					  pPixmap, pGC,
					  pBox->x1, pBox->y1,
 					  pBox->x2 - pBox->x1,
 					    pBox->y2 - pBox->y1,
					  pBox->x1 - x, pBox->y1 - y);
			else
			    (*pGC->ops->CopyPlane) (pBackingStore->pBackingPixmap,
			    		  pPixmap, pGC,
					  pBox->x1, pBox->y1,
 					  pBox->x2 - pBox->x1,
 					    pBox->y2 - pBox->y1,
					  pBox->x1 - x, pBox->y1 - y, planeMask);
			pBox++;
		    }
		}
	    }
	}
    }

    /*
     * draw the border of this window into the pixmap
     */

    /* XXX can't tile the border into the wrong depth pixmap */

    if (HasBorder (pWin) &&
	(pWin->drawable.depth == pPixmap->drawable.depth ||
	 pWin->borderIsPixel))
    {
	unsigned long	pixel;

	pBox = (*pScreen->RegionExtents) (pRgn);

	if (pBox->x1 < 0 || pBox->y1 < 0 ||
	    wBorderWidth (pWin) + (int) pWin->clientWinSize.width < pBox->x2 ||
	    wBorderWidth (pWin) + (int) pWin->clientWinSize.height < pBox->y2)
	{
	    /*
	     * compute areas of border to display
	     */

	    (*pScreen->Subtract) (pBackRgn, &pWin->borderSize, &pWin->winSize);

	    /*
	     * translate relative to pWin
	     */
	    (*pScreen->TranslateRegion) (pBackRgn,
 					 -pWin->absCorner.x,
 					 -pWin->absCorner.y);

	    /*
	     * extract regions to fill
	     */

	    (*pScreen->Intersect) (pBackRgn, pBackRgn, pRgn);

	    if ((*pScreen->RegionNotEmpty) (pBackRgn))
	    {
	        pixel = pWin->borderPixel;
	        if (pWin->drawable.depth != pPixmap->drawable.depth)
		    pixel = (pWin->borderPixel & planeMask) != 0;
	        miBSFillVirtualBits ((DrawablePtr)pPixmap, pGC, pBackRgn,
				     x, y,
				     pixel,
				     pWin->borderTile,
				     (PixmapPtr) USE_BORDER_PIXEL, planeMask);
	    }
	}
    }

    (*pScreen->RegionDestroy) (pBackRgn);

    /*
     * now fetch any bits from children's backing store
     */
    if (pWin->lastChild)
    {
	for (pChild = pWin->lastChild; pChild; pChild = pChild->prevSib) {
	    /*
	     * create a region which covers the non-visibile
	     * portions of the child which need to
	     * be restored
	     */

	    dx = pChild->drawable.x - pWin->drawable.x;
	    dy = pChild->drawable.y - pWin->drawable.y;

	    (*pScreen->TranslateRegion) (pRgn, -dx, -dy);

	    miBSDoGetImage (pChild, pPixmap, pRgn, x - dx, y - dy,
 	    		    pGC, planeMask);

	    (*pScreen->TranslateRegion) (pRgn, dx, dy);
	}
    }
}
#endif

/*-
 *-----------------------------------------------------------------------
 * miBSAllocate --
 *	Create and install backing store info for a window
 *
 *-----------------------------------------------------------------------
 */

static void
miBSAllocate(pWin)
    WindowPtr 	  pWin;
{
    register miBSWindowPtr  pBackingStore;
    register ScreenPtr 	    pScreen;
	
    if (!pWin->backStorage &&
	(pWin->drawable.pScreen->backingStoreSupport != NotUseful))
    {
	pScreen = pWin->drawable.pScreen;

	pBackingStore = (miBSWindowPtr)xalloc(sizeof(miBSWindowRec));
	if (!pBackingStore)
	    return;

	pBackingStore->pBackingPixmap = NullPixmap;
	(* pScreen->RegionInit)(&pBackingStore->pSavedRegion, NullBox, 1);
	pBackingStore->viewable = (char)pWin->viewable;
	pBackingStore->status = StatusNoPixmap;
	pBackingStore->backgroundState = None;
	
	pWin->backStorage = (pointer) pBackingStore;

	/*
	 * Now want to initialize the backing pixmap and pSavedRegion if
	 * necessary. The initialization consists of finding all the
	 * currently-obscured regions, by taking the inverse of the window's
	 * clip list, storing the result in pSavedRegion, and exposing those
	 * areas of the window.
	 */

	if ((pWin->backingStore == WhenMapped && pWin->viewable) ||
	    (pWin->backingStore == Always))
 	{
	    BoxPtr	pBox;
	    BoxRec  	box;
	    RegionPtr	pSavedRegion;
	    xEvent	*pEvent, *pe;
	    int		i;
	    int		numRects;

	    pSavedRegion = &pBackingStore->pSavedRegion;

	    box.x1 = pWin->drawable.x;
	    box.x2 = box.x1 + (int) pWin->drawable.width;
	    box.y1 = pWin->drawable.y;
	    box.y2 = pWin->drawable.y + (int) pWin->drawable.height;

	    (* pScreen->Inverse)(pSavedRegion, &pWin->clipList,  &box);
	    (* pScreen->TranslateRegion) (pSavedRegion,
					  -pWin->drawable.x,
					  -pWin->drawable.y);
#ifdef SHAPE
	    if (wBoundingShape (pWin))
		(*pScreen->Intersect) (pSavedRegion, pSavedRegion, wBoundingShape (pWin));
	    if (wClipShape (pWin))
		(*pScreen->Intersect) (pSavedRegion, pSavedRegion, wClipShape (pWin));
#endif
	    miTileVirtualBS (pWin);
	    
	    /*
	     * deliver all the newly availible regions
	     * as exposure events to the window
	     */

	    pBox = REGION_RECTS(pSavedRegion);
	    numRects = REGION_NUM_RECTS(pSavedRegion);
	    if(!(pEvent = (xEvent *) ALLOCATE_LOCAL(numRects *
						    sizeof(xEvent))))
		return;

	    for (i=numRects, pe = pEvent; --i >= 0; pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pWin->drawable.id;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = i;
	    }
	    DeliverEvents(pWin, pEvent, numRects, NullWindow);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSFree --
 *	Destroy and free all the stuff associated with the backing-store
 *	for the given window.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The backing pixmap and all the regions and GC's are destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFree(pWin)
    WindowPtr pWin;
{
    miBSWindowPtr 	pBackingStore;
    register ScreenPtr	pScreen = pWin->drawable.pScreen;

    if (pWin->backStorage)
    {
	pBackingStore = (miBSWindowPtr)pWin->backStorage;
	if (pBackingStore)
	{
	    miDestroyBSPixmap (pWin);
    
	    (* pScreen->RegionUninit)(&pBackingStore->pSavedRegion);

	    if (pBackingStore->backgroundState == BackgroundPixmap)
		(*pScreen->DestroyPixmap) (pBackingStore->background.pixmap);

	    xfree(pBackingStore);
	    pWin->backStorage = NULL;
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miResizeBackingStore --
 *	Alter the size of the backing pixmap when the window changes
 *	size. The contents of the old pixmap are copied into the new
 *	one displaced by the given amounts. When copying, copies the
 *	bounding box of the saved regions, on the assumption that that
 *	is faster than copying the component boxes...?
 *
 * Results:
 *	The new Pixmap.
 *
 * Side Effects:
 *	The old pixmap is destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
miResizeBackingStore(pWin, dx, dy)
    WindowPtr 	  pWin;
    int	    	  dx,
		  dy;
{
    miBSWindowPtr pBackingStore;
    PixmapPtr pBackingPixmap;
    ScreenPtr pScreen;
    GC	   *pGC;
    BoxPtr  extents;
    BoxRec pixbounds;
    RegionPtr prgnTmp;

    pBackingStore = (miBSWindowPtr)(pWin->backStorage);
    pScreen = pWin->drawable.pScreen;
    pBackingPixmap = pBackingStore->pBackingPixmap;

    if (pBackingPixmap)
    {
	PixmapPtr pNewPixmap;

	pNewPixmap = (PixmapPtr)(*pScreen->CreatePixmap)
					(pScreen, 
					 pWin->drawable.width, 
					 pWin->drawable.height, 
					 pWin->drawable.depth);

	if (pNewPixmap)
	{
	    if ((* pScreen->RegionNotEmpty) (&pBackingStore->pSavedRegion))
	    {
		extents = (*pScreen->RegionExtents)(&pBackingStore->pSavedRegion);
		pGC = GetScratchGC(pNewPixmap->drawable.depth, pScreen);
		if (pGC)
		{
		    ValidateGC((DrawablePtr)pNewPixmap, pGC);
		    (*pGC->ops->CopyArea)(pBackingPixmap, pNewPixmap, pGC,
				     extents->x1, extents->y1,
				     extents->x2 - extents->x1,
				     extents->y2 - extents->y1,
				     extents->x1 + dx, extents->y1 + dy);
		    FreeScratchGC(pGC);
		}
	    }
	}
	else
	{
	    pBackingStore->status = StatusNoPixmap;
	}

	(* pScreen->DestroyPixmap)(pBackingPixmap);
	pBackingStore->pBackingPixmap = pNewPixmap;
    }

    /*
     * Now we need to translate pSavedRegion, as appropriate, and clip it
     * to be within the window's new bounds.
     */
    if (dx || dy)
    {
	(* pWin->drawable.pScreen->TranslateRegion)
				(&pBackingStore->pSavedRegion, dx, dy);
    }
    pixbounds.x1 = 0;
    pixbounds.x2 = pWin->drawable.width;
    pixbounds.y1 = 0;
    pixbounds.y2 = pWin->drawable.height;
    prgnTmp = (* pScreen->RegionCreate)(&pixbounds, 1);
#ifdef SHAPE
    if (wBoundingShape (pWin))
	(*pScreen->Intersect) (prgnTmp, prgnTmp, wBoundingShape (pWin));
    if (wClipShape (pWin))
	(*pScreen->Intersect) (prgnTmp, prgnTmp, wClipShape (pWin));
#endif
    (* pScreen->Intersect)(&pBackingStore->pSavedRegion,
			   &pBackingStore->pSavedRegion,
			   prgnTmp);
    (* pScreen->RegionDestroy)(prgnTmp);
}

/*-
 *-----------------------------------------------------------------------
 * miBSSaveDoomedAreas --
 *	Saved the areas of the given window that are about to be
 *	obscured.  If the window has moved, pObscured is expected to
 *	be at the new location and (dx,dy) is expected to be the offset
 *	to the window's previous location.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The region is copied from the screen into pBackingPixmap and
 *	pSavedRegion is updated.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSSaveDoomedAreas(pWin, pObscured, dx, dy)
    register WindowPtr pWin;
    RegionPtr 	       pObscured;
    int		       dx, dy;
{
    miBSWindowPtr 	pBackingStore;
    ScreenPtr	  	pScreen;
    

    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pScreen = pWin->drawable.pScreen;

    /*
     * If the window isn't realized, it's being unmapped, thus we don't
     * want to save anything if backingStore isn't Always.
     */
    if (!pWin->realized)
    {
	pBackingStore->viewable = (char)pWin->viewable;
	if (pWin->backingStore != Always)
	{
	    (* pScreen->RegionEmpty) (&pBackingStore->pSavedRegion);
	    miDestroyBSPixmap (pWin);
	    return;
	}
    }

    /* Don't even pretend to save anything for a virtual background None */
    if ((pBackingStore->status == StatusVirtual) &&
	(pBackingStore->backgroundState == None))
	return;

    if ((*pScreen->RegionNotEmpty)(pObscured))
    {
	(*pScreen->TranslateRegion) (pObscured,
				     -pWin->drawable.x, -pWin->drawable.y);
	/*
	 * only save the bits if we've actually
	 * started using backing store
	 */
	if (pBackingStore->status != StatusVirtual)
	{
	    miBSScreenPtr	pScreenPriv;

	    pScreenPriv = (miBSScreenPtr) pScreen->devPrivates[miBSScreenIndex].ptr;
	    if (!pBackingStore->pBackingPixmap)
		miCreateBSPixmap (pWin);

	    if (pBackingStore->pBackingPixmap)
		(* pScreenPriv->funcs->SaveAreas) (pBackingStore->pBackingPixmap,
						   pObscured,
						   pWin->drawable.x - dx,
						   pWin->drawable.y - dy);
	}
	(* pScreen->Union)(&pBackingStore->pSavedRegion,
			   &pBackingStore->pSavedRegion,
			   pObscured);
	(*pScreen->TranslateRegion) (pObscured,
				     pWin->drawable.x, pWin->drawable.y);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSRestoreAreas --
 *	Restore areas from backing-store that are no longer obscured.
 *	expects prgnExposed to contain a screen-relative area.
 *
 * Results:
 *	The region to generate exposure events on (which may be
 *	different from the region to paint).
 *
 * Side Effects:
 *	Areas are copied from pBackingPixmap to the screen. prgnExposed
 *	is altered to contain the region that could not be restored from
 *	backing-store.
 *
 * Notes:
 *	This is called before sending any exposure events to the client,
 *	and so might be called if the window has grown.  Changing the backing
 *	pixmap doesn't require revalidating the backingGC because the
 *	client's next output request will result in a call to ValidateGC,
 *	since the window clip region has changed, which will in turn call
 *	miValidateBackingStore.
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSRestoreAreas(pWin, prgnExposed)
    register WindowPtr pWin;
    RegionPtr prgnExposed;
{
    PixmapPtr pBackingPixmap;
    miBSWindowPtr pBackingStore;
    RegionPtr prgnSaved;
    RegionPtr prgnRestored;
    register ScreenPtr pScreen;
    RegionPtr exposures = prgnExposed;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pBackingPixmap = pBackingStore->pBackingPixmap;

    prgnSaved = &pBackingStore->pSavedRegion;

    if (pBackingStore->status == StatusContents)
    {
	miBSScreenPtr	pScreenPriv;

	(*pScreen->TranslateRegion) (prgnSaved, pWin->drawable.x, pWin->drawable.y);

	prgnRestored = (* pScreen->RegionCreate)((BoxPtr)NULL, 1);
	(* pScreen->Intersect)(prgnRestored, prgnExposed, prgnSaved);
	
	/*
	 * Since prgnExposed is no longer obscured, we no longer
	 * will have a valid copy of it in backing-store, but there is a valid
	 * copy of it on screen, so subtract the area we just restored from
	 * from the area to be exposed.
	 */

	(* pScreen->Subtract)(prgnSaved, prgnSaved, prgnExposed);
	(* pScreen->Subtract)(prgnExposed, prgnExposed, prgnRestored);
	
	/*
	 * Do the actual restoration
	 */

	pScreenPriv = (miBSScreenPtr)
	    pScreen->devPrivates[miBSScreenIndex].ptr;
	(* pScreenPriv->funcs->RestoreAreas) (pBackingPixmap,
					 prgnRestored,
					 pWin->drawable.x,
					 pWin->drawable.y);
	
	(* pScreen->RegionDestroy)(prgnRestored);

	/*
	 * if the saved region is completely empty, dispose of the
	 * backing pixmap, otherwise, retranslate the saved
	 * region to window relative
	 */

	if (!(*pScreen->RegionNotEmpty) (prgnSaved))
	    miDestroyBSPixmap (pWin);
	else
	    (*pScreen->TranslateRegion) (prgnSaved,
					 -pWin->drawable.x, -pWin->drawable.y);
    }
    else if ((pBackingStore->status == StatusVirtual) ||
	     (pBackingStore->status == StatusVDirty))
    {
	(*pScreen->TranslateRegion) (prgnSaved,
				     pWin->drawable.x, pWin->drawable.y);
	exposures = (* pScreen->RegionCreate)(NullBox, 1);
	if (SameBackground (pBackingStore->backgroundState,
			    pBackingStore->background,
			    pWin->backgroundState,
 			    pWin->background))
	{
	    (* pScreen->Subtract)(exposures, prgnExposed, prgnSaved);
	}
	else
	{
	    /* background has changed, virtually retile and expose */
	    if (pBackingStore->backgroundState == BackgroundPixmap)
		(* pScreen->DestroyPixmap) (pBackingStore->background.pixmap);
	    miTileVirtualBS(pWin);

	    /* we need to expose all we have (virtually) retiled */
	    (* pScreen->Union) (exposures, prgnExposed, prgnSaved);
	}
	(* pScreen->Subtract)(prgnSaved, prgnSaved, prgnExposed);
	(*pScreen->TranslateRegion) (prgnSaved,
				     -pWin->drawable.x, -pWin->drawable.y);
    }
    else if (pWin->viewable && !pBackingStore->viewable &&
	     pWin->backingStore != Always)
    {
	/*
	 * The window was just mapped and nothing has been saved in
	 * backing-store from the last time it was mapped. We want to capture
	 * any output to regions that are already obscured but there are no
	 * bits to snag off the screen, so we initialize things just as we did
	 * in miBSAllocate, above.
	 */
	BoxRec  box;
	
	prgnSaved = &pBackingStore->pSavedRegion;

	box.x1 = pWin->drawable.x;
	box.x2 = box.x1 + (int) pWin->drawable.width;
	box.y1 = pWin->drawable.y;
	box.y2 = box.y1 + (int) pWin->drawable.height;
	
	(* pScreen->Inverse)(prgnSaved, &pWin->clipList,  &box);
	(* pScreen->TranslateRegion) (prgnSaved,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
#ifdef SHAPE
	if (wBoundingShape (pWin))
	    (*pScreen->Intersect) (prgnSaved, prgnSaved, wBoundingShape (pWin));
	if (wClipShape (pWin))
	    (*pScreen->Intersect) (prgnSaved, prgnSaved, wClipShape (pWin));
#endif
	miTileVirtualBS(pWin);

	exposures = (* pScreen->RegionCreate)(&box, 1);
    }
    pBackingStore->viewable = (char)pWin->viewable;
    return exposures;
}


/*-
 *-----------------------------------------------------------------------
 * miBSTranslateBackingStore --
 *	Shift the backing-store in the given direction. Called when bit
 *	gravity is shifting things around. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the window changed size as well as position, the backing pixmap
 *	is resized. The contents of the backing pixmap are shifted
 *
 *-----------------------------------------------------------------------
 */
static void
miBSTranslateBackingStore(pWin, dx, dy, oldClip)
    WindowPtr 	  pWin;
    int     	  dx;		/* translation distance */
    int     	  dy;
    RegionPtr	  oldClip;  	/* Region being copied */
{
    register miBSWindowPtr 	pBackingStore;
    register RegionPtr 	    	pSavedRegion;
    register RegionPtr 	    	newSaved, obscured;
    register ScreenPtr		pScreen;
    BoxRec			extents;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr)(pWin->backStorage);
    if (pBackingStore->status == StatusNoPixmap)
	return;

    /* bit gravity makes things virtually too hard, punt */
    if (((dx != 0) || (dy != 0)) && (pBackingStore->status != StatusContents))
	miCreateBSPixmap(pWin);

    pSavedRegion = &pBackingStore->pSavedRegion;
    if (!oldClip)
	(* pScreen->RegionEmpty) (pSavedRegion);
    newSaved = (* pScreen->RegionCreate) (NullBox, 1);
    obscured = (* pScreen->RegionCreate) (NullBox, 1);
    /* resize and translate backing pixmap and pSavedRegion */
    miResizeBackingStore(pWin, dx, dy);
    /* now find any already saved areas we should retain */
    if (pWin->viewable)
    {
	(* pScreen->RegionCopy) (newSaved, &pWin->clipList);
	(* pScreen->TranslateRegion)(newSaved, -dx, -dy);
	(* pScreen->Intersect) (pSavedRegion, pSavedRegion, newSaved);
    }
    /* compute what the new pSavedRegion will be */
    extents.x1 = pWin->drawable.x;
    extents.x2 = pWin->drawable.x + (int) pWin->drawable.width;
    extents.y1 = pWin->drawable.y;
    extents.y2 = pWin->drawable.y + (int) pWin->drawable.height;
    (* pScreen->Inverse)(newSaved, &pWin->clipList, &extents);
#ifdef SHAPE
    if (wBoundingShape (pWin) || wClipShape (pWin)) {
	(* pScreen->TranslateRegion) (newSaved,
				    -pWin->drawable.x,
				    -pWin->drawable.y);
	if (wBoundingShape (pWin))
	    (* pScreen->Intersect) (newSaved, newSaved, wBoundingShape (pWin));
	if (wClipShape (pWin))
	    (* pScreen->Intersect) (newSaved, newSaved, wClipShape (pWin));
	(* pScreen->TranslateRegion) (newSaved,
				    pWin->drawable.x,
				    pWin->drawable.y);
    }
#endif
    
    /* now find any visible areas we can save from the screen */
    (* pScreen->TranslateRegion)(newSaved, -dx, -dy);
    if (oldClip)
    {
	(* pScreen->Intersect) (obscured, oldClip, newSaved);
	if ((* pScreen->RegionNotEmpty) (obscured))
	{
	    /* save those visible areas */
	    (* pScreen->TranslateRegion) (obscured, dx, dy);
	    miBSSaveDoomedAreas(pWin, obscured, dx, dy);
	}
    }
    /* translate newSaved to local coordinates */
    (* pScreen->TranslateRegion) (newSaved,
				  dx-pWin->drawable.x,
				  dy-pWin->drawable.y);
    /* subtract out what we already have saved */
    (* pScreen->Subtract) (obscured, newSaved, pSavedRegion);
    /* and expose whatever there is */
    if ((* pScreen->RegionNotEmpty) (obscured))
    {
	RegionRec tmpRgn;
	extents = *((* pScreen->RegionExtents) (obscured));
	tmpRgn = pBackingStore->pSavedRegion; /* don't look */
	pBackingStore->pSavedRegion = *obscured;
	/* XXX there is a problem here.  The protocol requires that
	 * only the last exposure event in a series can have a zero
	 * count, but we might generate one here, and then another
	 * one will be generated if there are unobscured areas that
	 * are newly exposed.  Is it worth computing, or cheating?
	 */
	miBSClearBackingStore(pWin, extents.x1, extents.y1,
			      extents.x2 - extents.x1,
			      extents.y2 - extents.y1,
			      TRUE);
	pBackingStore->pSavedRegion = tmpRgn;
    }
    /* finally install new pSavedRegion */
    (* pScreen->Union) (pSavedRegion, pSavedRegion, newSaved);
    (* pScreen->RegionDestroy) (newSaved);
    (* pScreen->RegionDestroy) (obscured);
}

/*
 * Inform the backing store layer that you are about to validate
 * a gc with a window, and that subsequent output to the window
 * is (or is not) guaranteed to be already clipped to the visible
 * regions of the window.
 */

static void
miBSDrawGuarantee (pWin, pGC, guarantee)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		guarantee;
{
    miBSGCPtr 	pPriv;

    if (pWin->backStorage)
    {
	pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;
	if (!pPriv)
	    miBSCreateGCPrivate (pGC);
	pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;
	if (pPriv)
	{
	    /*
	     * XXX KLUDGE ALERT
	     *
	     * when the GC is Cheap pPriv will point
	     * at some device's gc func structure.  guarantee
	     * will point at the ChangeGC entry of that struct
	     * and will never match a valid guarantee value.
	     */
	    switch (pPriv->guarantee)
	    {
	    case GuaranteeNothing:
	    case GuaranteeVisBack:
		pPriv->guarantee = guarantee;
		break;
	    }
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSValidateGC --
 *	Wrapper around output-library's ValidateGC routine
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 * Notes:
 *	The idea here is to perform several functions:
 *	    - All the output calls must be intercepted and routed to
 *	      backing-store as necessary.
 *	    - pGC in the window's devBackingStore must be set up with the
 *	      clip list appropriate for writing to pBackingPixmap (i.e.
 *	      the inverse of the window's clipList intersected with the
 *	      clientClip of the GC). Since the destination for this GC is
 *	      a pixmap, it is sufficient to set the clip list as its
 *	      clientClip.
 *-----------------------------------------------------------------------
 */

static void
miBSValidateGC (pGC, stateChanges, pDrawable)
    GCPtr   	  pGC;
    unsigned long stateChanges;
    DrawablePtr   pDrawable;
{
    GCPtr   	  	pBackingGC;
    miBSWindowPtr	pWindowPriv;
    miBSGCPtr		pPriv;
    WindowPtr		pWin;
    int			lift_functions;
    RegionPtr		backingCompositeClip = NULL;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
        pWin = (WindowPtr) pDrawable;
	pWindowPriv = (miBSWindowPtr) pWin->backStorage;
	lift_functions = (pWindowPriv == (miBSWindowPtr) NULL);
    }
    else
    {
        pWin = (WindowPtr) NULL;
	lift_functions = TRUE;
    }

    pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);

    /*
     * rewrap funcs and ops as Validate may have changed them
     */

    pPriv->wrapFuncs = pGC->funcs;
    pPriv->wrapOps = pGC->ops;

    if (pPriv->guarantee == GuaranteeVisBack)
        lift_functions = TRUE;

    /*
     * check to see if a new backingCompositeClip region must
     * be generated
     */

    if (!lift_functions && 
        ((pDrawable->serialNumber != pPriv->serialNumber) ||
	 (stateChanges&(GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode))))
    {
	if ((*pGC->pScreen->RegionNotEmpty) (&pWindowPriv->pSavedRegion))
 	{
	    backingCompositeClip = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    if ((pGC->clientClipType == CT_NONE) || 
		(pGC->clientClipType == CT_PIXMAP))
	    {
		(*pGC->pScreen->RegionCopy) (backingCompositeClip,
					     &pWindowPriv->pSavedRegion); 
	    }
	    else
	    {
		/*
		 * Make a new copy of the client clip, translated to
		 * its proper origin.
		 */

		(*pGC->pScreen->RegionCopy) (backingCompositeClip, pGC->clientClip);
		(*pGC->pScreen->TranslateRegion) (backingCompositeClip,
						  pGC->clipOrg.x,
						  pGC->clipOrg.y);
		(*pGC->pScreen->Intersect) (backingCompositeClip, backingCompositeClip,
					    &pWindowPriv->pSavedRegion);
	    }
	    if (pGC->subWindowMode == IncludeInferiors)
 	    {
		RegionPtr translatedClip;

		/* XXX
		 * any output in IncludeInferiors mode will not
		 * be redirected to Inferiors backing store.  This
		 * can be fixed only at great cost to the shadow routines.
		 */
		translatedClip = NotClippedByChildren (pWin);
		(*pGC->pScreen->TranslateRegion) (translatedClip,
						  pGC->clipOrg.x,
						  pGC->clipOrg.y);
		(*pGC->pScreen->Subtract) (backingCompositeClip, backingCompositeClip, translatedClip);
		(*pGC->pScreen->RegionDestroy) (translatedClip);
	    }

	    if (!(*pGC->pScreen->RegionNotEmpty) (backingCompositeClip)) {
		lift_functions = TRUE;
	    }

	}
 	else
 	{
	    lift_functions = TRUE;
	}

	/* Reset the status when drawing to an unoccluded window so that
	 * future SaveAreas will actually copy bits from the screen.  Note that
	 * output to root window in IncludeInferiors mode will not cause this
	 * to change.  This causes all transient graphics by the window
	 * manager to the root window to not enable backing store.
	 */
	if (lift_functions && (pWindowPriv->status == StatusVirtual) &&
	    (pWin->parent || pGC->subWindowMode != IncludeInferiors))
	    pWindowPriv->status = StatusVDirty;
    }

    if (!lift_functions && !pWin->realized && pWin->backingStore != Always)
    {
	lift_functions = TRUE;
    }

    /*
     * if no backing store has been allocated, and it's needed,
     * create it now
     */

    if (!lift_functions && !pWindowPriv->pBackingPixmap)
    {
	miCreateBSPixmap (pWin);
	if (!pWindowPriv->pBackingPixmap)
	    lift_functions = TRUE;
    }
    
    /*
     * create the backing GC if needed, lift functions
     * if the creation fails
     */

    if (!lift_functions && !pPriv->pBackingGC)
    {
	int status;

	pBackingGC = CreateGC ((DrawablePtr)pWindowPriv->pBackingPixmap,
			       (BITS32)0, (XID *)NULL, &status);
	if (status != Success)
	    lift_functions = TRUE;
	else
	    pPriv->pBackingGC = pBackingGC;
    }

    pBackingGC = pPriv->pBackingGC;

    pPriv->stateChanges |= stateChanges;

    if (lift_functions)
    {
	if (backingCompositeClip)
	    (* pGC->pScreen->RegionDestroy) (backingCompositeClip);

	/*
	 * don't wrap the operations, just the functions
	 */

	pGC->funcs = &miBSGCFuncs;

	return;
    }

    /*
     * the rest of this function gets the pBackingGC
     * into shape for possible draws
     */

    CopyGC(pGC, pBackingGC, pPriv->stateChanges);

    pPriv->stateChanges = 0;

    /*
     * We never want operations with the backingGC to generate GraphicsExpose
     * events...
     */
    if (stateChanges & GCGraphicsExposures)
    {
	XID false = xFalse;

	DoChangeGC(pBackingGC, GCGraphicsExposures, &false, FALSE);
    }

    if (backingCompositeClip)
    {
	if (pGC->clientClipType == CT_PIXMAP)
	{
	    miBSScreenPtr   pScreenPriv;

	    pScreenPriv = (miBSScreenPtr) 
		pGC->pScreen->devPrivates[miBSScreenIndex].ptr;
	    (* pScreenPriv->funcs->SetClipmaskRgn)
		(pBackingGC, backingCompositeClip);
	    (* pGC->pScreen->RegionDestroy) (backingCompositeClip);
	}
	else
	{
	    (*pBackingGC->funcs->ChangeClip) (pBackingGC, CT_REGION, backingCompositeClip, 0);
	}
	pPriv->serialNumber = pDrawable->serialNumber;
    }
    
    if (pWindowPriv->pBackingPixmap->drawable.serialNumber
    	!= pBackingGC->serialNumber)
    {
	ValidateGC((DrawablePtr)pWindowPriv->pBackingPixmap, pBackingGC);
    }

    if (pBackingGC->clientClip == 0)
    	ErrorF ("backing store clip list nil");

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (*pGC->funcs->ChangeGC) (pGC, mask);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGCDst)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGCDst, pPriv);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

    FUNC_EPILOGUE (pGCDst, pPriv);
}

static void
miBSDestroyGC (pGC)
    GCPtr   pGC;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    if (pPriv->pBackingGC)
	FreeGC(pPriv->pBackingGC, (GContext)0);

    (*pGC->funcs->DestroyGC) (pGC);

    FUNC_EPILOGUE (pGC, pPriv);

    xfree(pPriv);
}

static void
miBSChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (* pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pgcDst)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pgcDst, pPriv);

    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    FUNC_EPILOGUE (pgcDst, pPriv);
}

static void
miBSDestroyClip(pGC)
    GCPtr	pGC;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (* pGC->funcs->DestroyClip)(pGC);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miDestroyBSPixmap (pWin)
    WindowPtr	pWin;
{
    miBSWindowPtr	pBackingStore;
    ScreenPtr		pScreen;
    
    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    if (pBackingStore->pBackingPixmap)
	(* pScreen->DestroyPixmap)(pBackingStore->pBackingPixmap);
    pBackingStore->pBackingPixmap = NullPixmap;
    if (pBackingStore->backgroundState == BackgroundPixmap)
	(* pScreen->DestroyPixmap)(pBackingStore->background.pixmap);
    pBackingStore->backgroundState = None;
    pBackingStore->status = StatusNoPixmap;
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
}

static void
miTileVirtualBS (pWin)
    WindowPtr	pWin;
{
    miBSWindowPtr	pBackingStore;

    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    pBackingStore->backgroundState = pWin->backgroundState;
    pBackingStore->background = pWin->background;
    if (pBackingStore->backgroundState == BackgroundPixmap)
	pBackingStore->background.pixmap->refcnt++;

    if (pBackingStore->status != StatusVDirty)
	pBackingStore->status = StatusVirtual;

    /*
     * punt parent relative tiles and do it now
     */
    if (pBackingStore->backgroundState == ParentRelative)
	miCreateBSPixmap (pWin);
}

#ifdef DEBUG
static int BSAllocationsFailed = 0;
#define FAILEDSIZE	32
static struct { int w, h; } failedRecord[FAILEDSIZE];
static int failedIndex;
#endif

static void
miCreateBSPixmap (pWin)
    WindowPtr	pWin;
{
    miBSWindowPtr	pBackingStore;
    ScreenPtr		pScreen;
    PixUnion		background;
    char		backgroundState;
    BoxPtr		extents;
    Bool		backSet;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    backSet = ((pBackingStore->status == StatusVirtual) ||
	       (pBackingStore->status == StatusVDirty));

    if (!pBackingStore->pBackingPixmap)
	pBackingStore->pBackingPixmap =
    	    (PixmapPtr)(* pScreen->CreatePixmap)
			   (pScreen,
			    pWin->drawable.width,
			    pWin->drawable.height,
			    pWin->drawable.depth);

    if (!pBackingStore->pBackingPixmap)
    {
#ifdef DEBUG
	BSAllocationsFailed++;
	/*
	 * record failed allocations
	 */
	failedRecord[failedIndex].w = pWin->drawable.width;
	failedRecord[failedIndex].h = pWin->drawable.height;
	failedIndex++;
	if (failedIndex == FAILEDSIZE)
		failedIndex = 0;
#endif
	pBackingStore->status = StatusNoPixmap;
	return; /* XXX */
    }

    pBackingStore->status = StatusContents;

    if (backSet)
    {
	backgroundState = pWin->backgroundState;
	background = pWin->background;
    
	pWin->backgroundState = pBackingStore->backgroundState;
	pWin->background = pBackingStore->background;
	if (pWin->backgroundState == BackgroundPixmap)
	    pWin->background.pixmap->refcnt++;
    }

    if ((* pScreen->RegionNotEmpty) (&pBackingStore->pSavedRegion))
    {
	extents = (* pScreen->RegionExtents) (&pBackingStore->pSavedRegion);
	miBSClearBackingStore(pWin,
			      extents->x1, extents->y1,
			      extents->x2 - extents->x1,
			      extents->y2 - extents->y1,
			      FALSE);
    }

    if (backSet)
    {
	if (pWin->backgroundState == BackgroundPixmap)
	    (* pScreen->DestroyPixmap) (pWin->background.pixmap);
	pWin->backgroundState = backgroundState;
	pWin->background = background;
	if (pBackingStore->backgroundState == BackgroundPixmap)
	    (* pScreen->DestroyPixmap) (pBackingStore->background.pixmap);
	pBackingStore->backgroundState = None;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSExposeCopy --
 *	Handle the restoration of areas exposed by graphics operations.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	prgnExposed has the areas exposed from backing-store removed
 *	from it.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSExposeCopy (pSrc, pDst, pGC, prgnExposed, srcx, srcy, dstx, dsty, plane)
    WindowPtr	  	pSrc;
    DrawablePtr	  	pDst;
    GCPtr   	  	pGC;
    RegionPtr	  	prgnExposed;
    int	    	  	srcx, srcy;
    int	    	  	dstx, dsty;
    unsigned long 	plane;
{
    RegionPtr	  	tempRgn;
    miBSWindowPtr	pBackingStore;
    RegionPtr	  	(*copyProc)();
    register BoxPtr	pBox;
    register int  	i;
    register int  	dx, dy;

    if (!(*pGC->pScreen->RegionNotEmpty) (prgnExposed))
	return;
    pBackingStore = (miBSWindowPtr)pSrc->backStorage;
    
    if (pBackingStore->status == StatusNoPixmap)
    	return;

    tempRgn = (* pGC->pScreen->RegionCreate) (NULL, 1);
    (* pGC->pScreen->Intersect) (tempRgn, prgnExposed,
				 &pBackingStore->pSavedRegion);
    (* pGC->pScreen->Subtract) (prgnExposed, prgnExposed, tempRgn);

    if (plane != 0) {
	copyProc = pGC->ops->CopyPlane;
    } else {
	copyProc = pGC->ops->CopyArea;
    }
    
    dx = dstx - srcx;
    dy = dsty - srcy;
    
    switch (pBackingStore->status) {
    case StatusVirtual:
    case StatusVDirty:
	pGC = GetScratchGC (pDst->depth, pDst->pScreen);
	if (pGC)
	{
	    miBSFillVirtualBits (pDst, pGC, tempRgn, dx, dy,
				 pBackingStore->backgroundState,
				 pBackingStore->background,
				 ~0L);
	    FreeScratchGC (pGC);
	}
	break;
    case StatusContents:
	for (i = REGION_NUM_RECTS(tempRgn), pBox = REGION_RECTS(tempRgn);
	     --i >= 0;
	     pBox++)
	{
	    (* copyProc) (pBackingStore->pBackingPixmap,
			  pDst, pGC, pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
	break;
    }
}
