/*
 * misprite.c
 *
 * machine independent software sprite routines
 */

/* $XConsortium: misprite.c,v 5.0 89/06/09 15:09:27 keith Exp $ */

# include   "X.h"
# include   "misc.h"
# include   "pixmapstr.h"
# include   "input.h"
# include   "mi.h"
# include   "cursorstr.h"
# include   "font.h"
# include   "scrnintstr.h"
# include   "windowstr.h"
# include   "gcstruct.h"
# include   "mipointer.h"
# include   "mispritestr.h"

/*
 * screen wrappers
 */

static int  miSpriteScreenIndex = -1;

static Bool	    miSpriteCloseScreen();
static void	    miSpriteGetImage();
static unsigned int *miSpriteGetSpans();
static Bool	    miSpriteCreateGC();
static Bool	    miSpriteCreateWindow();
static Bool	    miSpriteChangeWindowAttributes();
static void	    miSpriteBlockHandler();

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((miSpriteScreenPtr) (pScreen)->devPrivates[miSpriteScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/*
 * window wrappers
 */

static int  miSpriteWindowIndex = -1;

static void miSpritePaintWindowBackground(),	miSpritePaintWindowBorder();
static void miSpriteCopyWindow(),		miSpriteClearToBackground();

static WindowFuncs miSpriteWindowFuncs = {
    miSpritePaintWindowBackground,
    miSpritePaintWindowBorder,
    miSpriteCopyWindow,
    miSpriteClearToBackground,
};

#define WINDOW_PROLOGUE(pWin)\
    ((pWin)->funcs = \
	((miSpriteWindowPtr) (pWin->devPrivates[miSpriteWindowIndex].ptr))->wrapFuncs)
	
#define WINDOW_EPILOGUE(pWin)\
    ((pWin)->funcs = &miSpriteWindowFuncs)

/*
 * backing store wrappers
 */

static void	    miSpriteSaveDoomedAreas();
static RegionPtr    miSpriteRestoreAreas();
static void	    miSpriteExposeCopy(), miSpriteTranslateBackingStore();
static void	    miSpriteClearBSToBackground(), miSpriteDrawGuarantee();

static BackingStoreFuncs miSpriteBackingStoreFuncs = {
    miSpriteSaveDoomedAreas,
    miSpriteRestoreAreas,
    miSpriteExposeCopy,
    miSpriteTranslateBackingStore,
    miSpriteClearBSToBackground,
    miSpriteDrawGuarantee,
};

#define BSTORE_PROLOGUE(pWin) (\
  (WINDOW_PROLOGUE(pWin)) , \
  ((pWin)->backStorage->funcs = \
    ((miSpriteWindowPtr) (pWin)->devPrivates[miSpriteWindowIndex].ptr)->wrapBSFuncs))

#define BSTORE_EPILOGUE(pWin) (\
  (WINDOW_EPILOGUE(pWin)) , \
  ((pWin)->backStorage->funcs = &miSpriteBackingStoreFuncs))

/*
 * GC func wrappers
 */

static int  miSpriteGCIndex = -1;

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

#define GC_SETUP(pDrawable)					    \
    miSpriteScreenPtr	pScreenPriv = (miSpriteScreenPtr)	    \
	(pDrawable)->pScreen->devPrivates[miSpriteScreenIndex].ptr;

#define GC_SETUP_AND_CHECK(pDrawable)				    \
    GC_SETUP(pDrawable);					    \
    if (GC_CHECK((WindowPtr)pDrawable))				    \
	miSpriteRemoveCursor (pDrawable->pScreen);
    
#define GC_CHECK(pWin)						    \
    (pScreenPriv->isUp && (pWin)->viewable &&			    \
	(pWin)->drawable.x < pScreenPriv->saved.x2 &&		    \
	pScreenPriv->saved.x1 < (pWin)->drawable.x + (int) (pWin)->drawable.width && \
	(pWin)->drawable.y < pScreenPriv->saved.y2 &&		    \
	pScreenPriv->saved.y1 < (pWin)->drawable.y + (int) (pWin)->drawable.height)

#define GC_OP_PROLOGUE(pGC) (\
    ((pGC)->funcs = \
	((miSpriteGCPtr) (pGC)->devPrivates[miSpriteGCIndex].ptr)->wrapFuncs), \
    ((pGC)->ops = \
	((miSpriteGCPtr) (pGC)->devPrivates[miSpriteGCIndex].ptr)->wrapOps))

#define GC_OP_EPILOGUE(pGC) (\
    ((pGC)->funcs = &miSpriteGCFuncs), \
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
    
    if (miSpriteScreenIndex == -1)
    {
	miSpriteScreenIndex = AllocateScreenPrivateIndex ();
	if (miSpriteScreenIndex == -1)
	    return FALSE;
    }
    if (miSpriteWindowIndex == -1)
	miSpriteWindowIndex = AllocateWindowPrivateIndex ();
    if (miSpriteGCIndex == -1)
	miSpriteGCIndex = AllocateGCPrivateIndex ();
    pPriv = (miSpriteScreenPtr) xalloc (sizeof (miSpriteScreenRec));
    if (!pPriv)
	return FALSE;
    miPointerInitialize (pScreen, &miSpritePointerFuncs, pointerFuncs);
    pPriv->CloseScreen = pScreen->CloseScreen;
    pPriv->GetImage = pScreen->GetImage;
    pPriv->GetSpans = pScreen->GetSpans;
    pPriv->CreateGC = pScreen->CreateGC;
    pPriv->CreateWindow = pScreen->CreateWindow;
    pPriv->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pPriv->BlockHandler = pScreen->BlockHandler;
    pPriv->pCursor = NULL;
    pPriv->x = 0;
    pPriv->y = 0;
    pPriv->isUp = FALSE;
    pPriv->shouldBeUp = FALSE;
    pPriv->funcs = spriteFuncs;
    pScreen->devPrivates[miSpriteScreenIndex].ptr = (pointer) pPriv;
    pScreen->CloseScreen = miSpriteCloseScreen;
    pScreen->GetImage = miSpriteGetImage;
    pScreen->GetSpans = miSpriteGetSpans;
    pScreen->CreateGC = miSpriteCreateGC;
    pScreen->CreateWindow = miSpriteCreateWindow;
    pScreen->ChangeWindowAttributes = miSpriteChangeWindowAttributes;
    pScreen->BlockHandler = miSpriteBlockHandler;
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
    pScreen->CreateWindow = pScreenPriv->CreateWindow;
    pScreen->ChangeWindowAttributes = pScreenPriv->ChangeWindowAttributes;
    pScreen->CreateGC = pScreenPriv->CreateGC;

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
miSpriteCreateWindow (pWin)
    WindowPtr	    pWin;
{
    ScreenPtr		pScreen;
    miSpriteWindowPtr   pWindowPriv;
    Bool		ret;

    pScreen = pWin->drawable.pScreen;

    pWindowPriv = (miSpriteWindowPtr) xalloc (sizeof (miSpriteWindowRec));
    if (!pWindowPriv)
	return FALSE;

    SCREEN_PROLOGUE (pScreen, CreateWindow);

    ret = (*pScreen->CreateWindow) (pWin);

    if (ret)
    {
	if (pWin->backStorage)
	{
	    pWindowPriv->wrapBSFuncs = pWin->backStorage->funcs;
	    pWin->backStorage->funcs = &miSpriteBackingStoreFuncs;
	}

	pWindowPriv->wrapFuncs = pWin->funcs;
	pWin->funcs = &miSpriteWindowFuncs;
	pWin->devPrivates[miSpriteWindowIndex].ptr = (pointer) pWindowPriv;
    }
    else
    {
	pWin->devPrivates[miSpriteWindowIndex].ptr = (pointer) 0;
	xfree ((pointer) pWindowPriv);
    }

    SCREEN_EPILOGUE (pScreen, CreateWindow, miSpriteCreateWindow);

    return ret;
}

static Bool
miSpriteChangeWindowAttributes (pWin, mask)
    WindowPtr	    pWin;
    unsigned long   mask;
{
    ScreenPtr	    pScreen;
    miSpriteWindowPtr    pWindowPriv;
    Bool	ret;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, ChangeWindowAttributes);

    pWindowPriv = (miSpriteWindowPtr) pWin->devPrivates[miSpriteWindowIndex].ptr;
    /*
     * unwrap the window funcs before diving down
     */
    if (pWin->backStorage)
	pWin->backStorage->funcs = pWindowPriv->wrapBSFuncs;
    pWin->funcs = pWindowPriv->wrapFuncs;

    ret = (*pScreen->ChangeWindowAttributes) (pWin, mask);

    /*
     * rewrap window funcs
     */

    if (pWin->backStorage)
    {
	pWindowPriv->wrapBSFuncs = pWin->backStorage->funcs;
	pWin->backStorage->funcs = &miSpriteBackingStoreFuncs;
    }

    pWindowPriv->wrapFuncs = pWin->funcs;
    pWin->funcs = &miSpriteWindowFuncs;

    SCREEN_EPILOGUE (pScreen, ChangeWindowAttributes, miSpriteChangeWindowAttributes);

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
    
    pPriv = (miSpriteGCPtr) xalloc (sizeof (miSpriteGCRec));
    if (pPriv && (*pScreen->CreateGC) (pGC))
    {
	pPriv->wrapOps = pGC->ops;
	pPriv->wrapFuncs = pGC->funcs;
    	pGC->devPrivates[miSpriteGCIndex].ptr = (pointer) pPriv;
    	pGC->funcs = &miSpriteGCFuncs;
	pGC->ops = &miSpriteGCOps;
	ret = TRUE;
    }

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

/*
 * BackingStore wrappers
 */

static void
miSpriteSaveDoomedAreas (pWin)
    WindowPtr	pWin;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr   pScreenPriv;
    BoxRec		cursorBox;
    int			dx, dy;

    BSTORE_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;
    
    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	cursorBox = pScreenPriv->saved;

	dx = pWin->drawable.x - pWin->backStorage->oldAbsCorner.x;
	dy = pWin->drawable.y - pWin->backStorage->oldAbsCorner.y;

	if (dx || dy)
 	{
	    cursorBox.x1 += dx;
	    cursorBox.y1 += dy;
	    cursorBox.x2 += dx;
	    cursorBox.y2 += dy;
	}
	if ((* pScreen->RectIn) (pWin->backStorage->obscured, &cursorBox) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pWin->backStorage->funcs->SaveDoomedAreas) (pWin);

    BSTORE_EPILOGUE (pWin);
}

static RegionPtr
miSpriteRestoreAreas (pWin)
    WindowPtr	pWin;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr   pScreenPriv;
    BoxRec		cursorBox;
    RegionPtr		result;

    BSTORE_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;
    
    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	cursorBox = pScreenPriv->saved;
	/*
	 * The exposed region is now window-relative, so we have to make the
	 * cursor box window-relative too.
	 */
	cursorBox.x1 -= pWin->drawable.x;
	cursorBox.x2 -= pWin->drawable.x;
	cursorBox.y1 -= pWin->drawable.y;
	cursorBox.y2 -= pWin->drawable.y;
	if ((* pScreen->RectIn) (pWin->exposed, &cursorBox) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    result = (*pWin->backStorage->funcs->RestoreAreas) (pWin);

    BSTORE_EPILOGUE (pWin);

    return result;
}

static void
miSpriteExposeCopy(pSrc, pDst, pGC, prgnExposed, srcx, srcy, dstx, dsty, plane)
    WindowPtr	  	pSrc;
    DrawablePtr	  	pDst;
    GCPtr   	  	pGC;
    RegionPtr	  	prgnExposed;
    int	    	  	srcx, srcy;
    int	    	  	dstx, dsty;
    unsigned long 	plane;
{
    BSTORE_PROLOGUE (pSrc);

    (*pSrc->backStorage->funcs->ExposeCopy)
	(pSrc, pDst, pGC, prgnExposed, srcx, srcy, dstx, dsty, plane);

    BSTORE_EPILOGUE (pSrc);
}

static void
miSpriteTranslateBackingStore(pWin, dx, dy, oldClip)
    WindowPtr 	  pWin;
    int     	  dx;		/* translation distance */
    int     	  dy;
    RegionPtr	  oldClip;  	/* Region being copied */
{
    BSTORE_PROLOGUE (pWin);

    (*pWin->backStorage->funcs->TranslateBackingStore) (pWin, dx, dy, oldClip);

    BSTORE_EPILOGUE (pWin);
}

static void
miSpriteClearBSToBackground (pWin, x, y, w, h, generateExposures)
    WindowPtr	  	pWin;
    int	    	  	x;
    int	    	  	y;
    int	    	  	w;
    int	    	  	h;
    Bool    	  	generateExposures;
{
    BSTORE_PROLOGUE (pWin);

    (*pWin->backStorage->funcs->ClearToBackground)
	(pWin, x, y, w, h, generateExposures);

    BSTORE_EPILOGUE (pWin);
}

static void
miSpriteDrawGuarantee (pWin, pGC, guarantee)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		guarantee;
{
    GC_FUNC_PROLOGUE (pGC);
    BSTORE_PROLOGUE (pWin);

    (*pWin->backStorage->funcs->DrawGuarantee) (pWin, pGC, guarantee);
    
    BSTORE_EPILOGUE (pWin);
    GC_FUNC_EPILOGUE (pGC);
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

    WINDOW_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;

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

    (*pWin->funcs->PaintWindowBackground) (pWin, pRegion, what);

    WINDOW_EPILOGUE (pWin);
}

static void
miSpritePaintWindowBorder (pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    ScreenPtr	    pScreen;
    miSpriteScreenPtr    pScreenPriv;

    WINDOW_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;

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

    (*pWin->funcs->PaintWindowBorder) (pWin, pRegion, what);

    WINDOW_EPILOGUE (pWin);
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

    WINDOW_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;

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

    (*pWin->funcs->CopyWindow) (pWin, ptOldOrg, pRegion);

    WINDOW_EPILOGUE (pWin);
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

    WINDOW_PROLOGUE (pWin);

    pScreen = pWin->drawable.pScreen;
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

    (*pWin->funcs->ClearToBackground) (pWin, x, y, w, h, generateExposures);

    WINDOW_EPILOGUE (pWin);
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
    
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pGCPriv->wrapOps = pGC->ops;
    }
    else
    {
	/*
	 * leave the ops unwrapped
	 */
	pGCPriv->wrapOps = NULL;
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

    xfree ((pointer) pGCPriv);
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
    GC_SETUP(pDrawable);

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
    GC_SETUP(pDrawable);

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
    GC_SETUP(pDrawable);

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
    GC_SETUP(pDst);

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
	GC_SETUP(pSrc);

    	if (GC_CHECK((WindowPtr) pSrc) &&
	    ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))
    	{
	    miSpriteRemoveCursor (pSrc->pScreen);
    	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);

    GC_OP_EPILOGUE (pGC);
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
    GC_SETUP(pDst);

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
	GC_SETUP(pSrc);

    	if (GC_CHECK((WindowPtr) pSrc) &&
	    ORG_OVERLAP(&pScreenPriv->saved,pSrc->x,pSrc->y,srcx,srcy,w,h))
    	{
	    miSpriteRemoveCursor (pSrc->pScreen);
    	}
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

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
    GC_SETUP(pDrawable);

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
    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PolyFillArc) (pDrawable, pGC, narcs, parcs);

    GC_OP_EPILOGUE (pGC);
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

    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

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

    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

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
    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

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
    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->ImageText16) (pDrawable, pGC, x, y, count, chars);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->ImageGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpritePolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    GC_SETUP_AND_CHECK(pDrawable);

    GC_OP_PROLOGUE (pGC);

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
    GC_SETUP (pDrawable);

    if (GC_CHECK((WindowPtr) pDrawable) &&
	ORG_OVERLAP(&pScreenPriv->saved,pDrawable->x,pDrawable->y,x,y,w,h))
    {
	miSpriteRemoveCursor (pDrawable->pScreen);
    }

    GC_OP_PROLOGUE (pGC);

    (*pGC->ops->PushPixels) (pGC, pBitMap, pDrawable, w, h, x, y);

    GC_OP_EPILOGUE (pGC);
}

static void
miSpriteLineHelper(pDrawable, pGC, cap, npts, pts, xOrg, yOrg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    Bool	cap;
    int		npts;
    pointer	pts;
    int		xOrg, yOrg;
{
    GC_SETUP_AND_CHECK(pDrawable);

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
	pScreenPriv->pCursor == pCursor)
    {
	return;
    }
    pScreenPriv->x = x;
    pScreenPriv->y = y;
    pScreenPriv->pCursor = pCursor;
    if (pScreenPriv->isUp) {
	int	sx, sy;
	/*
	 * check to see if the old saved region completely
	 * encloses the new sprite, in which case we use
	 * the flicker-free MoveCursor primitive.
	 */
	sx = pScreenPriv->x - pCursor->xhot;
	sy = pScreenPriv->y - pCursor->yhot;
	if (sx >= pScreenPriv->saved.x1 &&
	    sx + pCursor->width < pScreenPriv->saved.x2 &&
	    sy >= pScreenPriv->saved.y1 &&
	    sy + pCursor->height < pScreenPriv->saved.y2)
	{
	    pScreenPriv->isUp = FALSE;
	    (void) (*pScreenPriv->funcs->MoveCursor) (pScreen, pCursor,
				  pScreenPriv->saved.x1,
 				  pScreenPriv->saved.y1,
				  pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				  pScreenPriv->saved.y2 - pScreenPriv->saved.y1,
				  sx - pScreenPriv->saved.x1,
				  sy - pScreenPriv->saved.y1);
	    pScreenPriv->isUp = TRUE;
	}
	else
	{
	    /*
	     * let the block handler put the cursor up
	     */
	    miSpriteRemoveCursor (pScreen);
	}
    }
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
    x = pScreenPriv->x - pCursor->xhot;
    y = pScreenPriv->y - pCursor->yhot;
    w = pCursor->width;
    h = pCursor->height;
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
	if ((*pScreenPriv->funcs->PutUpCursor) (pScreen, pCursor, x, y))
	    pScreenPriv->isUp = TRUE;
    }
}
