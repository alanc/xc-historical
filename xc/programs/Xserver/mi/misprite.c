/*
 * misprite.c
 *
 * machine independent software sprite routines
 */

/* $XConsortium: misprite.c,v 5.12 89/07/17 10:54:03 rws Exp $ */

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

# include   "X.h"
# include   "Xproto.h"
# include   "misc.h"
# include   "pixmapstr.h"
# include   "input.h"
# include   "mi.h"
# include   "cursorstr.h"
# include   "font.h"
# include   "scrnintstr.h"
# include   "colormapst.h"
# include   "windowstr.h"
# include   "gcstruct.h"
# include   "mipointer.h"
# include   "mispritest.h"
# include   "dixfontstr.h"
# include   "fontstruct.h"

/*
 * screen wrappers
 */

static int  miSpriteScreenIndex;
static unsigned long miSpriteGeneration = 0;

static Bool	    miSpriteCloseScreen();
static void	    miSpriteGetImage();
static unsigned int *miSpriteGetSpans();
static Bool	    miSpriteCreateGC();
static void	    miSpriteBlockHandler();
static void	    miSpriteInstallColormap();
static void	    miSpriteStoreColors();

static void	    miSpritePaintWindowBackground();
static void	    miSpritePaintWindowBorder();
static void	    miSpriteCopyWindow();
static void	    miSpriteClearToBackground();

static void	    miSpriteSaveDoomedAreas();
static RegionPtr    miSpriteRestoreAreas();

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((miSpriteScreenPtr) (pScreen)->devPrivates[miSpriteScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/*
 * GC func wrappers
 */

static int  miSpriteGCIndex;

static void miSpriteValidateGC (),  miSpriteCopyGC ();
static void miSpriteDestroyGC(),    miSpriteChangeGC();
static void miSpriteChangeClip(),   miSpriteDestroyClip();
static void miSpriteCopyClip();

static GCFuncs	miSpriteGCFuncs = {
    miSpriteValidateGC,
    miSpriteChangeGC,
    miSpriteCopyGC,
    miSpriteDestroyGC,
    miSpriteChangeClip,
    miSpriteDestroyClip,
    miSpriteCopyClip,
};

#define GC_FUNC_PROLOGUE(pGC)					\
    miSpriteGCPtr   pGCPriv =					\
	(miSpriteGCPtr) (pGC)->devPrivates[miSpriteGCIndex].ptr;\
    (pGC)->funcs = pGCPriv->wrapFuncs;				\
    if (pGCPriv->wrapOps)					\
	(pGC)->ops = pGCPriv->wrapOps;

#define GC_FUNC_EPILOGUE(pGC)					\
    pGCPriv->wrapFuncs = (pGC)->funcs;				\
    (pGC)->funcs = &miSpriteGCFuncs;				\
    if (pGCPriv->wrapOps)					\
    {								\
	pGCPriv->wrapOps = (pGC)->ops;				\
	(pGC)->ops = &miSpriteGCOps;				\
    }

/*
 * GC op wrappers
 */

static void	    miSpriteFillSpans(),	miSpriteSetSpans();
static void	    miSpritePutImage();
static RegionPtr    miSpriteCopyArea(),		miSpriteCopyPlane();
static void	    miSpritePolyPoint(),	miSpritePolylines();
static void	    miSpritePolySegment(),	miSpritePolyRectangle();
static void	    miSpritePolyArc(),		miSpriteFillPolygon();
static void	    miSpritePolyFillRect(),	miSpritePolyFillArc();
static int	    miSpritePolyText8(),	miSpritePolyText16();
static void	    miSpriteImageText8(),	miSpriteImageText16();
static void	    miSpriteImageGlyphBlt(),	miSpritePolyGlyphBlt();
static void	    miSpritePushPixels(),	miSpriteLineHelper();
static void	    miSpriteChangeClip(),	miSpriteDestroyClip();
static void	    miSpriteCopyClip();

static GCOps miSpriteGCOps = {
    miSpriteFillSpans,	    miSpriteSetSpans,	    miSpritePutImage,	
    miSpriteCopyArea,	    miSpriteCopyPlane,	    miSpritePolyPoint,
    miSpritePolylines,	    miSpritePolySegment,    miSpritePolyRectangle,
    miSpritePolyArc,	    miSpriteFillPolygon,    miSpritePolyFillRect,
    miSpritePolyFillArc,    miSpritePolyText8,	    miSpritePolyText16,
    miSpriteImageText8,	    miSpriteImageText16,    miSpriteImageGlyphBlt,
    miSpritePolyGlyphBlt,   miSpritePushPixels,	    miSpriteLineHelper,
};

/*
 * testing only -- remove cursor for every draw.  Eventually,
 * each draw operation will perform a bounding box check against
 * the saved cursor area
 */

#define GC_SETUP_CHEAP(pDrawable)				    \
    miSpriteScreenPtr	pScreenPriv = (miSpriteScreenPtr)	    \
	(pDrawable)->pScreen->devPrivates[miSpriteScreenIndex].ptr; \

#define GC_SETUP(pDrawable, pGC)				    \
    GC_SETUP_CHEAP(pDrawable)					    \
    miSpriteGCPtr	pGCPrivate = (miSpriteGCPtr)		    \
	(pGC)->devPrivates[miSpriteGCIndex].ptr;		    \
    GCFuncs *oldFuncs = pGC->funcs;

#define GC_SETUP_AND_CHECK(pDrawable, pGC)			    \
    GC_SETUP(pDrawable, pGC);					    \
    if (GC_CHECK((WindowPtr)pDrawable))				    \
	miSpriteRemoveCursor (pDrawable->pScreen);
    
#define GC_CHECK(pWin)						    \
    (pScreenPriv->isUp &&					    \
	(pWin)->drawable.x < pScreenPriv->saved.x2 &&		    \
	pScreenPriv->saved.x1 < (pWin)->drawable.x + (int) (pWin)->drawable.width && \
	(pWin)->drawable.y < pScreenPriv->saved.y2 &&		    \
	pScreenPriv->saved.y1 < (pWin)->drawable.y + (int) (pWin)->drawable.height)

#define GC_OP_PROLOGUE(pGC) (\
    ((pGC)->funcs = pGCPrivate->wrapFuncs), \
    ((pGC)->ops = pGCPrivate->wrapOps))

#define GC_OP_EPILOGUE(pGC) (\
    ((pGC)->funcs = oldFuncs), \
    ((pGC)->ops = &miSpriteGCOps))

/*
 * pointer-sprite method table
 */

static Bool miSpriteRealizeCursor (),	miSpriteUnrealizeCursor ();
static void miSpriteDisplayCursor (),	miSpriteUndisplayCursor ();

miPointerSpriteFuncRec miSpritePointerFuncs = {
    miSpriteRealizeCursor,
    miSpriteUnrealizeCursor,
    miSpriteDisplayCursor,
    miSpriteUndisplayCursor
};

/*
 * other misc functions
 */

static void miSpriteRemoveCursor (),	miSpriteRestoreCursor();

/*
 * miSpriteInitialize -- called from device-dependent screen
 * initialization proc after all of the function pointers have
 * been stored in the screen structure.
 */

Bool
miSpriteInitialize (pScreen, spriteFuncs, pointerFuncs)
    ScreenPtr		    pScreen;
    miSpriteCursorFuncPtr   spriteFuncs;
    miPointerCursorFuncPtr  pointerFuncs;
{
    miSpriteScreenPtr	pPriv;
    VisualPtr		pVisual;
    int			i;
    
    if (miSpriteGeneration != serverGeneration)
    {
	miSpriteScreenIndex = AllocateScreenPrivateIndex ();
	if (miSpriteScreenIndex < 0)
	    return FALSE;
	miSpriteGeneration = serverGeneration;
	miSpriteGCIndex = AllocateGCPrivateIndex ();
    }
    if (!AllocateGCPrivate(pScreen, miSpriteGCIndex, sizeof(miSpriteGCRec)))
	return FALSE;
    pPriv = (miSpriteScreenPtr) xalloc (sizeof (miSpriteScreenRec));
    if (!pPriv)
	return FALSE;
    if (!miPointerInitialize (pScreen, &miSpritePointerFuncs, pointerFuncs))
    {
	xfree ((pointer) pPriv);
	return FALSE;
    }
    pVisual = pScreen->visuals;
    for (i = 0; i < pScreen->numVisuals; i++)
	if (pVisual->vid == pScreen->rootVisual)
	    break;
    pPriv->pVisual = pVisual;
    pPriv->CloseScreen = pScreen->CloseScreen;
    pPriv->GetImage = pScreen->GetImage;
    pPriv->GetSpans = pScreen->GetSpans;
    pPriv->CreateGC = pScreen->CreateGC;
    pPriv->BlockHandler = pScreen->BlockHandler;
    pPriv->InstallColormap = pScreen->InstallColormap;
    pPriv->StoreColors = pScreen->StoreColors;

    pPriv->PaintWindowBackground = pScreen->PaintWindowBackground;
    pPriv->PaintWindowBorder = pScreen->PaintWindowBorder;
    pPriv->CopyWindow = pScreen->CopyWindow;
    pPriv->ClearToBackground = pScreen->ClearToBackground;

    pPriv->SaveDoomedAreas = pScreen->SaveDoomedAreas;
    pPriv->RestoreAreas = pScreen->RestoreAreas;

    pPriv->pCursor = NULL;
    pPriv->x = 0;
    pPriv->y = 0;
    pPriv->isUp = FALSE;
    pPriv->shouldBeUp = FALSE;
    pPriv->checkPixels = TRUE;
    pPriv->funcs = spriteFuncs;
    pScreen->devPrivates[miSpriteScreenIndex].ptr = (pointer) pPriv;
    pScreen->CloseScreen = miSpriteCloseScreen;
    pScreen->GetImage = miSpriteGetImage;
    pScreen->GetSpans = miSpriteGetSpans;
    pScreen->CreateGC = miSpriteCreateGC;
    pScreen->BlockHandler = miSpriteBlockHandler;
    pScreen->InstallColormap = miSpriteInstallColormap;
    pScreen->StoreColors = miSpriteStoreColors;

    pScreen->PaintWindowBackground = miSpritePaintWindowBackground;
    pScreen->PaintWindowBorder = miSpritePaintWindowBorder;
    pScreen->CopyWindow = miSpriteCopyWindow;
    pScreen->ClearToBackground = miSpriteClearToBackground;

    pScreen->SaveDoomedAreas = miSpriteSaveDoomedAreas;
    pScreen->RestoreAreas = miSpriteRestoreAreas;

    return TRUE;
}

/*
 * Screen wrappers
 */

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
miSpriteCloseScreen (i, pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->CreateGC = pScreenPriv->CreateGC;
    pScreen->BlockHandler = pScreenPriv->BlockHandler;
    pScreen->InstallColormap = pScreenPriv->InstallColormap;
    pScreen->StoreColors = pScreenPriv->StoreColors;

    pScreen->PaintWindowBackground = pScreenPriv->PaintWindowBackground;
    pScreen->PaintWindowBorder = pScreenPriv->PaintWindowBorder;
    pScreen->CopyWindow = pScreenPriv->CopyWindow;
    pScreen->ClearToBackground = pScreenPriv->ClearToBackground;

    pScreen->SaveDoomedAreas = pScreenPriv->SaveDoomedAreas;
    pScreen->RestoreAreas = pScreenPriv->RestoreAreas;

    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
miSpriteGetImage (pDrawable, sx, sy, w, h, format, planemask, pdstLine)
    DrawablePtr	    pDrawable;
    int		    sx, sy, w, h;
    unsigned int    format;
    unsigned long   planemask;
    pointer	    pdstLine;
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr    pScreenPriv;
    
    SCREEN_PROLOGUE (pScreen, GetImage);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    if (pDrawable->type == DRAWABLE_WINDOW &&
        pScreenPriv->isUp &&
	ORG_OVERLAP(&pScreenPriv->saved,pDrawable->x,pDrawable->y, sx, sy, w, h))
    {
	miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			  format, planemask, pdstLine);

    SCREEN_EPILOGUE (pScreen, GetImage, miSpriteGetImage);
}

static unsigned int *
miSpriteGetSpans (pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr	pDrawable;
    int		wMax;
    DDXPointPtr	ppt;
    int		*pwidth;
    int		nspans;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    unsigned int	    *ret;
    
    SCREEN_PROLOGUE (pScreen, GetSpans);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    if (pDrawable->type == DRAWABLE_WINDOW && pScreenPriv->isUp)
    {
	register DDXPointPtr    pts;
	register int    	*widths;
	register int    	nPts;
	register int    	xorg,
				yorg;

	xorg = pDrawable->x;
	yorg = pDrawable->y;

	for (pts = ppt, widths = pwidth, nPts = nspans;
	     nPts--;
	     pts++, widths++)
 	{
	    if (SPN_OVERLAP(&pScreenPriv->saved,pts->y+yorg,
			     pts->x+xorg,*widths))
	    {
		miSpriteRemoveCursor (pScreen);
		break;
	    }
	}
    }

    ret = (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans);

    SCREEN_EPILOGUE (pScreen, GetSpans, miSpriteGetSpans);

    return ret;
}

static Bool
miSpriteCreateGC (pGC)
    GCPtr   pGC;
{
    ScreenPtr	    pScreen = pGC->pScreen;
    Bool	    ret = FALSE;
    miSpriteGCPtr   pPriv;

    SCREEN_PROLOGUE (pScreen, CreateGC);
    
    pPriv = (miSpriteGCPtr)pGC->devPrivates[miSpriteGCIndex].ptr;

    ret = (*pScreen->CreateGC) (pGC);

    pPriv->wrapOps = NULL;
    pPriv->wrapFuncs = pGC->funcs;
    pGC->funcs = &miSpriteGCFuncs;

    SCREEN_EPILOGUE (pScreen, CreateGC, miSpriteCreateGC);

    return ret;
}

static void
miSpriteBlockHandler (i, blockData, pTimeout, pReadmask)
    int	i;
    pointer	blockData;
    pointer	pTimeout;
    pointer	pReadmask;
{
    ScreenPtr		pScreen = screenInfo.screens[i];
    miSpriteScreenPtr	pPriv;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, BlockHandler);
    
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);

    SCREEN_EPILOGUE(pScreen, BlockHandler, miSpriteBlockHandler);

    if (!pPriv->isUp && pPriv->shouldBeUp)
	miSpriteRestoreCursor (pScreen);
}

static void
miSpriteInstallColormap (pMap)
    ColormapPtr	pMap;
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, InstallColormap);
    
    (*pScreen->InstallColormap) (pMap);

    SCREEN_EPILOGUE(pScreen, InstallColormap, miSpriteInstallColormap);

    pPriv->pColormap = pMap;
    pPriv->checkPixels = TRUE;
}

static void
miSpriteStoreColors (pMap, ndef, pdef)
    ColormapPtr	pMap;
    int		ndef;
    xColorItem	*pdef;
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;
    int			i;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, StoreColors);
    
    (*pScreen->StoreColors) (pMap, ndef, pdef);

    SCREEN_EPILOGUE(pScreen, StoreColors, miSpriteStoreColors);

    if (pPriv->pColormap == pMap)
    {
	for (i = 0; i < ndef; i++)
	    /*
	     * XXX direct color will affect pixels other than
	     * pdef[i].pixel -- this will be more difficult...
	     */
	    if (pdef[i].pixel == pPriv->colors[SOURCE_COLOR].pixel ||
	        pdef[i].pixel == pPriv->colors[MASK_COLOR].pixel)
	    {
		pPriv->checkPixels = TRUE;
		if (pPriv->isUp && pPriv->shouldBeUp)
		    miSpriteRemoveCursor (pScreen);
		break;
	    }
    }
}

static void
miSpriteFindColors (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr	pScreenPriv = (miSpriteScreenPtr)
			    pScreen->devPrivates[miSpriteScreenIndex].ptr;
    CursorPtr		pCursor;
    xColorItem		*sourceColor, *maskColor;
    unsigned short	red, green, blue;
    Bool		read_only;

    pCursor = pScreenPriv->pCursor;
    sourceColor = &pScreenPriv->colors[SOURCE_COLOR];
    maskColor = &pScreenPriv->colors[MASK_COLOR];
    if (!(pCursor->foreRed == sourceColor->red &&
	  pCursor->foreGreen == sourceColor->green &&
          pCursor->foreBlue == sourceColor->blue &&
	  pCursor->backRed == maskColor->red &&
	  pCursor->backGreen == maskColor->green &&
	  pCursor->backBlue == maskColor->blue))
    {
	red = sourceColor->red = pCursor->foreRed;
	green = sourceColor->green = pCursor->foreGreen;
	blue = sourceColor->blue = pCursor->foreBlue;
	FakeAllocColor (pScreenPriv->pColormap, &red, &green, &blue,
			&sourceColor->pixel, &read_only);
	red = maskColor->red = pCursor->backRed;
	green = maskColor->green = pCursor->backGreen;
	blue = maskColor->blue = pCursor->backBlue;
	FakeAllocColor (pScreenPriv->pColormap, &red, &green, &blue,
			&maskColor->pixel, &read_only);
    }
    pScreenPriv->checkPixels = FALSE;
}

/*
 * BackingStore wrappers
 */

static void
miSpriteSaveDoomedAreas (pWin, pObscured, dx, dy)
    WindowPtr	pWin;
    RegionPtr	pObscured;
    int		dx, dy;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr   pScreenPriv;
    BoxRec		cursorBox;

    pScreen = pWin->drawable.pScreen;
    
    SCREEN_PROLOGUE (pScreen, SaveDoomedAreas);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	cursorBox = pScreenPriv->saved;

	if (dx || dy)
 	{
	    cursorBox.x1 += dx;
	    cursorBox.y1 += dy;
	    cursorBox.x2 += dx;
	    cursorBox.y2 += dy;
	}
	if ((* pScreen->RectIn) (pObscured, &cursorBox) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->SaveDoomedAreas) (pWin, pObscured, dx, dy);

    SCREEN_EPILOGUE (pScreen, SaveDoomedAreas, miSpriteSaveDoomedAreas);
}

static RegionPtr
miSpriteRestoreAreas (pWin, prgnExposed)
    WindowPtr	pWin;
    RegionPtr	prgnExposed;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr   pScreenPriv;
    RegionPtr		result;

    pScreen = pWin->drawable.pScreen;
    
    SCREEN_PROLOGUE (pScreen, RestoreAreas);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	if ((* pScreen->RectIn) (prgnExposed, &pScreenPriv->saved) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    result = (*pScreen->RestoreAreas) (pWin, prgnExposed);

    SCREEN_EPILOGUE (pScreen, RestoreAreas, miSpriteRestoreAreas);

    return result;
}

/*
 * Window wrappers
 */

static void
miSpritePaintWindowBackground (pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    ScreenPtr	    pScreen;
    miSpriteScreenPtr    pScreenPriv;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, PaintWindowBackground);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	/*
	 * If the cursor is on the same screen as the window, check the
	 * region to paint for the cursor and remove it as necessary
	 */
	if ((* pScreen->RectIn) (pRegion, &pScreenPriv->saved) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->PaintWindowBackground) (pWin, pRegion, what);

    SCREEN_EPILOGUE (pScreen, PaintWindowBackground, miSpritePaintWindowBackground);
}

static void
miSpritePaintWindowBorder (pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    ScreenPtr	    pScreen;
    miSpriteScreenPtr    pScreenPriv;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, PaintWindowBorder);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	/*
	 * If the cursor is on the same screen as the window, check the
	 * region to paint for the cursor and remove it as necessary
	 */
	if ((* pScreen->RectIn) (pRegion, &pScreenPriv->saved) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->PaintWindowBorder) (pWin, pRegion, what);

    SCREEN_EPILOGUE (pScreen, PaintWindowBorder, miSpritePaintWindowBorder);
}

static void
miSpriteCopyWindow (pWin, ptOldOrg, pRegion)
    WindowPtr	pWin;
    DDXPointRec	ptOldOrg;
    RegionPtr	pRegion;
{
    ScreenPtr	    pScreen;
    miSpriteScreenPtr    pScreenPriv;
    BoxRec	    cursorBox;
    int		    dx, dy;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, CopyWindow);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	/*
	 * check both the source and the destination areas.  The given
	 * region is source relative, so offset the cursor box by
	 * the delta position
	 */
	cursorBox = pScreenPriv->saved;
	dx = pWin->drawable.x - ptOldOrg.x;
	dy = pWin->drawable.y - ptOldOrg.y;
	cursorBox.x1 -= dx;
	cursorBox.x2 -= dx;
	cursorBox.y1 -= dy;
	cursorBox.y2 -= dy;
	if ((* pScreen->RectIn) (pRegion, &pScreenPriv->saved) != rgnOUT ||
	    (* pScreen->RectIn) (pRegion, &cursorBox) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->CopyWindow) (pWin, ptOldOrg, pRegion);

    SCREEN_EPILOGUE (pScreen, CopyWindow, miSpriteCopyWindow);
}

static void
miSpriteClearToBackground (pWin, x, y, w, h, generateExposures)
    WindowPtr pWin;
    short x,y;
    unsigned short w,h;
    Bool generateExposures;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr	pScreenPriv;
    int			realw, realh;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, ClearToBackground);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	if (!(realw = w))
	    realw = (int) pWin->drawable.width - x;
	if (!(realh = h))
	    realh = (int) pWin->drawable.height - y;
	if (ORG_OVERLAP(&pScreenPriv->saved, pWin->drawable.x, pWin->drawable.y,
			x, y, realw, realh))
	{
	    miSpriteRemoveCursor (pScreen);
	}
    }

    (*pScreen->ClearToBackground) (pWin, x, y, w, h, generateExposures);

    SCREEN_EPILOGUE (pScreen, ClearToBackground, miSpriteClearToBackground);
}

/*
 * GC Func wrappers
 */

static void
miSpriteValidateGC (pGC, changes, pDrawable)
    GCPtr	pGC;
    Mask	changes;
    DrawablePtr	pDrawable;
{
    GC_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ValidateGC) (pGC, changes, pDrawable);
    
    pGCPriv->wrapOps = NULL;
    if (pDrawable->type == DRAWABLE_WINDOW && ((WindowPtr) pDrawable)->viewable)
    {
	WindowPtr   pWin;
	RegionPtr   pRegion;

	pWin = (WindowPtr) pDrawable;
	pRegion = &pWin->clipList;
	if (pGC->subWindowMode == IncludeInferiors)
	    pRegion = &pWin->borderClip;
	if ((*pDrawable->pScreen->RegionNotEmpty) (pRegion))
	    pGCPriv->wrapOps = pGC->ops;
    }

    GC_FUNC_EPILOGUE (pGC);
}

static void
miSpriteChangeGC (pGC, mask)
    GCPtr	    pGC;
    unsigned long   mask;
{
    GC_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeGC) (pGC, mask);
    
    GC_FUNC_EPILOGUE (pGC);
}

static void
miSpriteCopyGC (pGCSrc, mask, pGCDst)
    GCPtr	    pGCSrc, pGCDst;
    unsigned long   mask;
{
    GC_FUNC_PROLOGUE (pGCDst);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    
    GC_FUNC_EPILOGUE (pGCDst);
}

static void
miSpriteDestroyGC (pGC)
    GCPtr   pGC;
{
    GC_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->DestroyGC) (pGC);
    
    GC_FUNC_EPILOGUE (pGC);
}

static void
miSpriteChangeClip (pGC, type, pvalue, nrects)
    GCPtr   pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    GC_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);

    GC_FUNC_EPILOGUE (pGC);
}

static void
miSpriteCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    GC_FUNC_PROLOGUE (pgcDst);

    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    GC_FUNC_EPILOGUE (pgcDst);
}

static void
miSpriteDestroyClip(pGC)
    GCPtr	pGC;
{
    GC_FUNC_PROLOGUE (pGC);

    (* pGC->funcs->DestroyClip)(pGC);

    GC_FUNC_EPILOGUE (pGC);
}

/*
 * GC Op wrappers
 */

static void
miSpriteFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	register DDXPointPtr    pts;
	register int    	*widths;
	register int    	nPts;

	for (pts = pptInit, widths = pwidthInit, nPts = nInit;
	     nPts--;
	     pts++, widths++)
 	{
	     if (SPN_OVERLAP(&pScreenPriv->saved,pts->y,pts->x,*widths))
	     {
		 miSpriteRemoveCursor (pDrawable->pScreen);
		 break;
	     }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->FillSpans) (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	register DDXPointPtr    pts;
	register int    	*widths;
	register int    	nPts;

	for (pts = ppt, widths = pwidth, nPts = nspans;
	     nPts--;
	     pts++, widths++)
 	{
	     if (SPN_OVERLAP(&pScreenPriv->saved,pts->y,pts->x,*widths))
	     {
		 miSpriteRemoveCursor(pDrawable->pScreen);
		 break;
	     }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->SetSpans) (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits)
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
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	if (ORG_OVERLAP(&pScreenPriv->saved,pDrawable->x,pDrawable->y,
			x,y,w,h))
 	{
	    miSpriteRemoveCursor (pDrawable->pScreen);
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PutImage) (pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits);

    GC_OP_EPILOGUE (pGC);
}

static RegionPtr
miSpriteCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
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
    RegionPtr rgn;

    GC_SETUP(pDst, pGC);

    /*
     * check both destination and source for overlap.  Avoid
     * the source check when source == destination
     */
    if (GC_CHECK((WindowPtr) pDst) && (
	 ORG_OVERLAP(&pScreenPriv->saved,pDst->x,pDst->y,dstx,dsty,w,h) ||
	 (pSrc == pDst &&
	  ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))))
    {
	miSpriteRemoveCursor (pDst->pScreen);
    }
    else if (pSrc != pDst && pSrc->type == DRAWABLE_WINDOW)
    {
	GC_SETUP_CHEAP(pSrc);

    	if (GC_CHECK((WindowPtr) pSrc) &&
	    ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))
    	{
	    miSpriteRemoveCursor (pSrc->pScreen);
    	}
    }

    GC_OP_PROLOGUE (pGC);

    rgn = (*pGC->ops->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h,
				 dstx, dsty);

    GC_OP_EPILOGUE (pGC);

    return rgn;
}

static RegionPtr
miSpriteCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
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
    RegionPtr rgn;

    GC_SETUP(pDst, pGC);

    /*
     * check both destination and source for overlap.  Avoid
     * the source check when source == destination
     */
    if (GC_CHECK((WindowPtr) pDst) && (
	 ORG_OVERLAP(&pScreenPriv->saved,pDst->x,pDst->y,dstx,dsty,w,h) ||
	 (pSrc == pDst &&
	  ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))))
    {
	miSpriteRemoveCursor (pDst->pScreen);
    }
    else if (pSrc != pDst && pSrc->type == DRAWABLE_WINDOW)
    {
	GC_SETUP_CHEAP(pSrc);

    	if (GC_CHECK((WindowPtr) pSrc) &&
	    ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))
    	{
	    miSpriteRemoveCursor (pSrc->pScreen);
    	}
    }

    GC_OP_PROLOGUE (pGC);

    rgn = (*pGC->ops->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
				  dstx, dsty, plane);

    GC_OP_EPILOGUE (pGC);

    return rgn;
}

/*
 * return true if the cursor overlaps the given set of points
 */

static Bool
miSpritePointOverlap (xorg, yorg, mode, lw, pPts, nPts, pCursorBox)
    register int  	    xorg;   	    /* X-Origin for points */
    register int  	    yorg;   	    /* Y-Origin for points */
    int	    	  	    mode;   	    /* CoordModeOrigin or
					     * CoordModePrevious */
    int			    lw;		    /* line width */
    register DDXPointPtr    pPts;   	    /* Array of points */
    int	    	  	    nPts;   	    /* Number of points */
    register BoxPtr 	    pCursorBox;	    /* Bounding box for cursor */
{
    register int  	    minx,
			    miny,
			    maxx,
			    maxy;

    minx = maxx = pPts->x + xorg;
    miny = maxy = pPts->y + yorg;

    pPts++;
    nPts--;

    if (mode == CoordModeOrigin) {
	while (nPts--) {
	    minx = min(minx, pPts->x + xorg);
	    maxx = max(maxx, pPts->x + xorg);
	    miny = min(miny, pPts->y + yorg);
	    maxy = max(maxy, pPts->y + yorg);
	    pPts++;
	}
    } else {
	xorg = minx;
	yorg = miny;
	while (nPts--) {
	    minx = min(minx, pPts->x + xorg);
	    maxx = max(maxx, pPts->x + xorg);
	    miny = min(miny, pPts->y + yorg);
	    maxy = max(maxy, pPts->y + yorg);
	    xorg += pPts->x;
	    yorg += pPts->y;
	    pPts++;
	}
    }
    lw = lw >> 1;
    return BOX_OVERLAP(pCursorBox,minx-lw,miny-lw,maxx+lw,maxy+lw);
}

static void
miSpritePolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    xPoint	t;
    int		i;
    BoxRec	cursor;

    GC_SETUP (pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	cursor.x1 = pScreenPriv->saved.x1 - pDrawable->x;
	cursor.y1 = pScreenPriv->saved.y1 - pDrawable->y;
	cursor.x2 = pScreenPriv->saved.x2 - pDrawable->x;
	cursor.y2 = pScreenPriv->saved.y2 - pDrawable->y;

	if (mode == CoordModePrevious)
	{
	    t.x = 0;
	    t.y = 0;
	    for (i = 0; i < npt; i++)
	    {
		t.x += pptInit[i].x;
		t.y += pptInit[i].y;
		if (cursor.x1 <= t.x && t.x <= cursor.x2 &&
		    cursor.y1 <= t.y && t.y <= cursor.y2)
		{
		    miSpriteRemoveCursor (pDrawable->pScreen);
		    break;
		}
	    }
	}
	else
	{
	    for (i = 0; i < npt; i++)
	    {
		if (cursor.x1 <= pptInit[i].x && pptInit[i].x <= cursor.x2 &&
		    cursor.y1 <= pptInit[i].y && pptInit[i].y <= cursor.y2)
		{
		    miSpriteRemoveCursor (pDrawable->pScreen);
		    break;
		}
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    GC_SETUP (pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	miSpritePointOverlap (pDrawable->x, pDrawable->y, mode,
			      (int)pGC->lineWidth, pptInit, npt,
			      &pScreenPriv->saved))
    {
	miSpriteRemoveCursor (pDrawable->pScreen);
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->Polylines) (pDrawable, pGC, mode, npt, pptInit);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolySegment(pDrawable, pGC, nseg, pSegs)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    int	    i;
    BoxPtr  cursor;
    int	    x, y, w, h;
    int	    lw;

    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	cursor = &pScreenPriv->saved;

	lw = (pGC->lineWidth + 1) / 2;
	for (i = 0; i < nseg; i++)
	{
	    if (pSegs[i].x1 <= pSegs[i].x2)
	    {
		x = pSegs[i].x1 - lw;
		w = pSegs[i].x2 - pSegs[i].x1 + lw;
	    }
	    if (pSegs[i].y1 <= pSegs[i].y2)
	    {
		y = pSegs[i].y1 - lw;
		h = pSegs[i].y2 - pSegs[i].y1 + lw;
	    }
	    if (ORG_OVERLAP (cursor, pDrawable->x, pDrawable->y,
			     x, y, w, h))
	    {
		miSpriteRemoveCursor (pDrawable->pScreen);
		break;
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolySegment) (pDrawable, pGC, nseg, pSegs);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyRectangle(pDrawable, pGC, nrects, pRects)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    BoxPtr  cursor;
    int	    lw;
    int	    i;
    
    GC_SETUP (pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	lw = (pGC->lineWidth + 1) / 2;
	cursor = &pScreenPriv->saved;
	for (i = 0; i < nrects; i++)
	{
	    if (ORG_OVERLAP (cursor, pDrawable->x, pDrawable->y,
			     pRects[i].x - lw, pRects[i].y - lw,
			     (int) pRects[i].width + lw,
 			     (int) pRects[i].height + lw))
	    {
		miSpriteRemoveCursor (pDrawable->pScreen);
		break;
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyRectangle) (pDrawable, pGC, nrects, pRects);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    BoxPtr  cursor;
    int	    lw;
    int	    i;
    
    GC_SETUP (pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	lw = (pGC->lineWidth + 1) / 2;
	cursor = &pScreenPriv->saved;
	for (i = 0; i < narcs; i++)
	{
	    if (ORG_OVERLAP (cursor, pDrawable->x, pDrawable->y,
			     parcs[i].x - lw, parcs[i].y - lw,
			     (int) parcs[i].width + lw,
 			     (int) parcs[i].height + lw))
	    {
		miSpriteRemoveCursor (pDrawable->pScreen);
		break;
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyArc) (pDrawable, pGC, narcs, parcs);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteFillPolygon(pDrawable, pGC, shape, mode, count, pPts)
    DrawablePtr		pDrawable;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    GC_SETUP (pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	miSpritePointOverlap (pDrawable->x, pDrawable->y, mode, 0,
			      pPts, count, &pScreenPriv->saved))
    {
	miSpriteRemoveCursor (pDrawable->pScreen);
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->FillPolygon) (pDrawable, pGC, shape, mode, count, pPts);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	register int	    nRect;
	register xRectangle *pRect;
	register int	    xorg, yorg;

	xorg = pDrawable->x;
	yorg = pDrawable->y;

	for (nRect = nrectFill, pRect = prectInit; nRect--; pRect++) {
	    if (ORGRECT_OVERLAP(&pScreenPriv->saved,xorg,yorg,pRect)){
		miSpriteRemoveCursor(pDrawable->pScreen);
		break;
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyFillRect) (pDrawable, pGC, nrectFill, prectInit);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyFillArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
    {
	register int	i;
	BoxPtr		cursor;

	cursor = &pScreenPriv->saved;

	for (i = 0; i < narcs; i++)
	{
	    if (ORG_OVERLAP(cursor, pDrawable->x, pDrawable->y,
			    parcs[i].x, parcs[i].y,
 			    (int) parcs[i].width, (int) parcs[i].height))
	    {
		miSpriteRemoveCursor (pDrawable->pScreen);
		break;
	    }
	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyFillArc) (pDrawable, pGC, narcs, parcs);

    GC_OP_EPILOGUE (pGC);
}

/*
 * general Poly/Image text function.  Extract glyph information,
 * compute bounding box and remove cursor if it is overlapped.
 */

static Bool
miSpriteTextOverlap (pDraw, font, x, y, n, charinfo, imageblt, w, cursorBox)
    DrawablePtr   pDraw;
    FontPtr	  font;
    int		  x, y;
    unsigned long n;
    CharInfoPtr   *charinfo;
    Bool	  imageblt;
    unsigned int  w;
    BoxPtr	  cursorBox;
{
    ExtentInfoRec extents;

    x += pDraw->x;
    y += pDraw->y;

    if (font->pFI->minbounds.metrics.characterWidth >= 0)
    {
	/* compute an approximate (but covering) bounding box */
	if (!imageblt || (charinfo[0]->metrics.leftSideBearing < 0))
	    extents.overallLeft = charinfo[0]->metrics.leftSideBearing;
	else
	    extents.overallLeft = 0;
	if (w)
	    extents.overallRight = w - charinfo[n-1]->metrics.characterWidth;
	else
	    extents.overallRight = font->pFI->maxbounds.metrics.characterWidth
				    * (n - 1);
	if (imageblt && (charinfo[n-1]->metrics.characterWidth >
			 charinfo[n-1]->metrics.rightSideBearing))
	    extents.overallRight += charinfo[n-1]->metrics.characterWidth;
	else
	    extents.overallRight += charinfo[n-1]->metrics.rightSideBearing;
	if (imageblt &&
	    (font->pFI->fontAscent > font->pFI->maxbounds.metrics.ascent))
	    extents.overallAscent = font->pFI->fontAscent;
	else
	    extents.overallAscent = font->pFI->maxbounds.metrics.ascent;
	if (imageblt &&
	    (font->pFI->fontDescent > font->pFI->maxbounds.metrics.descent))
	    extents.overallDescent = font->pFI->fontDescent;
	else
	    extents.overallDescent = font->pFI->maxbounds.metrics.descent;
	if (!BOX_OVERLAP(cursorBox,
			 x + extents.overallLeft,
			 y - extents.overallAscent,
			 x + extents.overallRight,
			 y + extents.overallDescent))
	    return FALSE;
	else if (imageblt && w)
	    return TRUE;
	/* if it does overlap, fall through and compute exactly, because
	 * taking down the cursor is expensive enough to make this worth it
	 */
    }
    QueryGlyphExtents(font, charinfo, n, &extents);
    if (imageblt)
    {
	if (extents.overallWidth > extents.overallRight)
	    extents.overallRight = extents.overallWidth;
	if (extents.overallLeft > 0)
	    extents.overallLeft = 0;
	if (extents.fontAscent > extents.overallAscent)
	    extents.overallAscent = extents.fontAscent;
	if (extents.fontDescent > extents.overallDescent)
	    extents.overallDescent = extents.fontDescent;
    }
    return (BOX_OVERLAP(cursorBox,
			x + extents.overallLeft,
			y - extents.overallAscent,
			x + extents.overallRight,
			y + extents.overallDescent));
}

static int 
miSpriteText (pDraw, pGC, x, y, count, chars, fontEncoding, imageblt, cursorBox)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    int		    x,
		    y;
    unsigned long    count;
    char	    *chars;
    FontEncoding    fontEncoding;
    Bool	    imageblt;
    BoxPtr	    cursorBox;
{
    CharInfoPtr *charinfo;
    unsigned long n, i;
    unsigned int w;
    void    	  (*drawFunc)();

    charinfo = (CharInfoPtr *) ALLOCATE_LOCAL(count * sizeof(CharInfoPtr));
    if (!charinfo)
	return x;

    GetGlyphs(pGC->font, count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    if (!imageblt)
	for (i = 0; i < n; i++)
	    w += charinfo[i]->metrics.characterWidth;

    if (n != 0) {
	if (miSpriteTextOverlap(pDraw, pGC->font, x, y, n, charinfo, imageblt, w, cursorBox))
	    miSpriteRemoveCursor(pDraw->pScreen);

	drawFunc = imageblt ? pGC->ops->ImageGlyphBlt : pGC->ops->PolyGlyphBlt;
	(*drawFunc) (pDraw, pGC, x, y, n, charinfo, pGC->font->pGlyphs);
    }
    DEALLOCATE_LOCAL(charinfo);
    return x + w;
}

static int
miSpritePolyText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    int	ret;

    GC_SETUP (pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
	ret = miSpriteText (pDrawable, pGC, x, y, (unsigned long)count, chars,
			    Linear8Bit, FALSE, &pScreenPriv->saved);
    else
	ret = (*pGC->ops->PolyText8) (pDrawable, pGC, x, y, count, chars);

    GC_OP_EPILOGUE (pGC);
    return ret;
}

static int
miSpritePolyText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    int	ret;

    GC_SETUP(pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
	ret = miSpriteText (pDrawable, pGC, x, y, (unsigned long)count,
			    (char *)chars,
			    pGC->font->pFI->lastRow == 0 ?
			    Linear16Bit : TwoD16Bit, FALSE, &pScreenPriv->saved);
    else
	ret = (*pGC->ops->PolyText16) (pDrawable, pGC, x, y, count, chars);

    GC_OP_EPILOGUE (pGC);
    return ret;
}

static void
miSpriteImageText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    GC_SETUP(pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
	(void) miSpriteText (pDrawable, pGC, x, y, count, chars, Linear8Bit, TRUE, &pScreenPriv->saved);
    else
	(*pGC->ops->ImageText8) (pDrawable, pGC, x, y, count, chars);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteImageText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    GC_SETUP(pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable))
	(void) miSpriteText (pDrawable, pGC, x, y, count, (char *)chars,
			    pGC->font->pFI->lastRow == 0 ?
			    Linear16Bit : TwoD16Bit, TRUE, &pScreenPriv->saved);
    else
	(*pGC->ops->ImageText16) (pDrawable, pGC, x, y, count, chars);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned long nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    GC_SETUP(pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	miSpriteTextOverlap (pDrawable, pGC->font, x, y, nglyph, ppci, TRUE, 0, &pScreenPriv->saved))
    {
	miSpriteRemoveCursor(pDrawable->pScreen);
    }
    (*pGC->ops->ImageGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned long nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    GC_SETUP (pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	miSpriteTextOverlap (pDrawable, pGC->font, x, y, nglyph, ppci, FALSE, 0, &pScreenPriv->saved))
    {
	miSpriteRemoveCursor(pDrawable->pScreen);
    }
    (*pGC->ops->PolyGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePushPixels(pGC, pBitMap, pDrawable, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDrawable;
    int		w, h, x, y;
{
    GC_SETUP(pDrawable, pGC);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	ORG_OVERLAP(&pScreenPriv->saved,pDrawable->x,pDrawable->y,x,y,w,h))
    {
	miSpriteRemoveCursor (pDrawable->pScreen);
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PushPixels) (pGC, pBitMap, pDrawable, w, h, x, y);

    GC_OP_EPILOGUE (pGC);
}

/*
 * I don't expect this routine will ever be called, as the GC
 * will have been unwrapped for the line drawing
 */

static void
miSpriteLineHelper(pDrawable, pGC, cap, npts, pts, xOrg, yOrg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    Bool	cap;
    int		npts;
    pointer	pts;
    int		xOrg, yOrg;
{
    GC_SETUP_AND_CHECK(pDrawable, pGC);

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->LineHelper) (pDrawable, pGC, cap, npts, pts, xOrg, yOrg);

    GC_OP_EPILOGUE (pGC);
}

/*
 * miPointer interface routines
 */

static Bool
miSpriteRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    return (*pScreenPriv->funcs->RealizeCursor) (pScreen, pCursor);
}

static Bool
miSpriteUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    return (*pScreenPriv->funcs->UnrealizeCursor) (pScreen, pCursor);
}

static void
miSpriteDisplayCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pScreenPriv->shouldBeUp = TRUE;
    if (pScreenPriv->x == x &&
	pScreenPriv->y == y &&
	pScreenPriv->pCursor == pCursor &&
	!pScreenPriv->checkPixels)
    {
	return;
    }
    pScreenPriv->x = x;
    pScreenPriv->y = y;
    if (pScreenPriv->checkPixels || pScreenPriv->pCursor != pCursor)
    {
	pScreenPriv->pCursor = pCursor;
	miSpriteFindColors (pScreen);
    }
    else
	pScreenPriv->pCursor = pCursor;
    if (pScreenPriv->isUp) {
	int	sx, sy;
	/*
	 * check to see if the old saved region completely
	 * encloses the new sprite, in which case we use
	 * the flicker-free MoveCursor primitive.
	 */
	sx = pScreenPriv->x - (int)pCursor->bits->xhot;
	sy = pScreenPriv->y - (int)pCursor->bits->yhot;
	if (sx >= pScreenPriv->saved.x1 &&
	    sx + (int)pCursor->bits->width < pScreenPriv->saved.x2 &&
	    sy >= pScreenPriv->saved.y1 &&
	    sy + (int)pCursor->bits->height < pScreenPriv->saved.y2)
	{
	    pScreenPriv->isUp = FALSE;
	    (void) (*pScreenPriv->funcs->MoveCursor) (pScreen, pCursor,
				  pScreenPriv->saved.x1,
 				  pScreenPriv->saved.y1,
				  pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				  pScreenPriv->saved.y2 - pScreenPriv->saved.y1,
				  sx - pScreenPriv->saved.x1,
				  sy - pScreenPriv->saved.y1,
				  pScreenPriv->colors[SOURCE_COLOR].pixel,
				  pScreenPriv->colors[MASK_COLOR].pixel);
	    pScreenPriv->isUp = TRUE;
	}
	else
	{
	    miSpriteRemoveCursor (pScreen);
	}
    }
    if (!pScreenPriv->isUp && pScreenPriv->pCursor)
	miSpriteRestoreCursor (pScreen);
}

/*ARGSUSED*/
static void
miSpriteUndisplayCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr   pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pScreenPriv->shouldBeUp = FALSE;
    if (pScreenPriv->isUp)
	miSpriteRemoveCursor (pScreen);
}

/*
 * undraw/draw cursor
 */

static void
miSpriteRemoveCursor (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pScreenPriv->isUp = FALSE;
    if (!(*pScreenPriv->funcs->RestoreUnderCursor) (pScreen,
					 pScreenPriv->saved.x1,
					 pScreenPriv->saved.y1,
					 pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
					 pScreenPriv->saved.y2 - pScreenPriv->saved.y1))
    {
	pScreenPriv->isUp = TRUE;
    }
}

/*
 * Called from the block handler, restores the cursor
 * before waiting for something to do.
 */

static void
miSpriteRestoreCursor (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;
    int		    x, y, w, h;
    CursorPtr	    pCursor;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pCursor = pScreenPriv->pCursor;
    x = pScreenPriv->x - (int)pCursor->bits->xhot;
    y = pScreenPriv->y - (int)pCursor->bits->yhot;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    pScreenPriv->saved.x1 = x - w/2;
    pScreenPriv->saved.y1 = y - h/2;
    pScreenPriv->saved.x2 = pScreenPriv->saved.x1 + w * 2;
    pScreenPriv->saved.y2 = pScreenPriv->saved.y1 + h * 2;
    if ((*pScreenPriv->funcs->SaveUnderCursor) (pScreen,
				      pScreenPriv->saved.x1,
				      pScreenPriv->saved.y1,
				      pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				      pScreenPriv->saved.y2 - pScreenPriv->saved.y1))
    {
	if (pScreenPriv->checkPixels)
	    miSpriteFindColors (pScreen);
	if ((*pScreenPriv->funcs->PutUpCursor) (pScreen, pCursor, x, y,
				  pScreenPriv->colors[SOURCE_COLOR].pixel,
				  pScreenPriv->colors[MASK_COLOR].pixel))
	    pScreenPriv->isUp = TRUE;
    }
}
