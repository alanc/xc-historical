/*
 * midispcur.c
 *
 * machine independent cursor display routines
 */

/* $XConsortium: midispcur.c,v 5.1 89/06/16 16:56:55 keith Exp $ */

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

#define NEED_EVENTS
# include   "X.h"
# include   "misc.h"
# include   "input.h"
# include   "cursorstr.h"
# include   "windowstr.h"
# include   "regionstr.h"
# include   "dixstruct.h"
# include   "scrnintstr.h"
# include   "servermd.h"
# include   "misprite.h"
# include   "mipointer.h"
# include   "gcstruct.h"

extern WindowPtr    *WindowTable;

/* per-screen private data */

static int	miDCScreenIndex = -1;

static Bool	miDCCloseScreen();

typedef struct {
    GCPtr	    pSourceGC, pMaskGC;
    GCPtr	    pSaveGC, pRestoreGC;
    GCPtr	    pMoveGC;
    GCPtr	    pPixSourceGC, pPixMaskGC;
    Bool	    (*CloseScreen)();
    PixmapPtr	    pSave, pTemp;
} miDCScreenRec, *miDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr		sourceBits;	    /* source bits */
    PixmapPtr		maskBits;	    /* mask bits */
} miDCCursorRec, *miDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool	miDCRealizeCursor(),	    miDCUnrealizeCursor();
static Bool	miDCPutUpCursor(),	    miDCSaveUnderCursor();
static Bool	miDCRestoreUnderCursor(),   miDCMoveCursor();

static miSpriteCursorFuncRec miDCFuncs = {
    miDCRealizeCursor,
    miDCUnrealizeCursor,
    miDCPutUpCursor,
    miDCSaveUnderCursor,
    miDCRestoreUnderCursor,
    miDCMoveCursor,
};

Bool
miDCInitialize (pScreen, cursorFuncs)
    ScreenPtr		    pScreen;
    miPointerCursorFuncPtr  cursorFuncs;
{
    miDCScreenPtr   pScreenPriv;

    if (miDCScreenIndex == -1)
    {
	miDCScreenIndex = AllocateScreenPrivateIndex ();
	if (miDCScreenIndex == -1)
	    return FALSE;
    }
    pScreenPriv = (miDCScreenPtr) xalloc (sizeof (miDCScreenRec));
    if (!pScreenPriv)
	return FALSE;

    /*
     * initialize the entire private structure to zeros
     */

    pScreenPriv->pSourceGC =
	pScreenPriv->pMaskGC =
	pScreenPriv->pSaveGC =
 	pScreenPriv->pRestoreGC =
 	pScreenPriv->pMoveGC =
 	pScreenPriv->pPixSourceGC =
	pScreenPriv->pPixMaskGC = NULL;
    
    pScreenPriv->pSave = pScreenPriv->pTemp = NULL;

    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = miDCCloseScreen;
    
    pScreen->devPrivates[miDCScreenIndex].ptr = (pointer) pScreenPriv;

    if (!miSpriteInitialize (pScreen, &miDCFuncs, cursorFuncs))
    {
	xfree ((pointer) pScreenPriv);
	return FALSE;
    }
    return TRUE;
}

#define tossGC(gc)  (gc ? FreeGC (gc, (GContext) 0) : 0)
#define tossPix(pix)	(pix ? (*pScreen->DestroyPixmap) (pix) : TRUE)

static Bool
miDCCloseScreen (index, pScreen)
    ScreenPtr	pScreen;
{
    miDCScreenPtr   pScreenPriv;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    tossGC (pScreenPriv->pSourceGC);
    tossGC (pScreenPriv->pMaskGC);
    tossGC (pScreenPriv->pSaveGC);
    tossGC (pScreenPriv->pRestoreGC);
    tossGC (pScreenPriv->pMoveGC);
    tossGC (pScreenPriv->pPixSourceGC);
    tossGC (pScreenPriv->pPixMaskGC);
    tossPix (pScreenPriv->pSave);
    tossPix (pScreenPriv->pTemp);
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
miDCRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miDCCursorPtr   pPriv;
    GCPtr	    pGC;
    XID		    gcvals[3];

    pPriv = (miDCCursorPtr) xalloc (sizeof (miDCCursorRec));
    if (!pPriv)
	return FALSE;
    pPriv->sourceBits = (*pScreen->CreatePixmap) (pScreen, pCursor->width, pCursor->height, 1);
    if (!pPriv->sourceBits)
    {
	xfree ((pointer) pPriv);
	return FALSE;
    }
    pPriv->maskBits =  (*pScreen->CreatePixmap) (pScreen, pCursor->width, pCursor->height, 1);
    if (!pPriv->maskBits)
    {
	(*pScreen->DestroyPixmap) (pPriv->sourceBits);
	xfree ((pointer) pPriv);
	return FALSE;
    }
    pCursor->devPriv[pScreen->myNum] = (pointer) pPriv;

    /* create the two sets of bits, clipping as appropriate */

    pGC = GetScratchGC (1, pScreen);
    if (!pGC)
    {
	(void) miDCUnrealizeCursor (pScreen, pCursor);
	return FALSE;
    }

    (*pGC->ops->PutImage) (pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->width, pCursor->height,
 			   0, XYPixmap, pCursor->source);
    gcvals[0] = GXand;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC (pPriv->sourceBits, pGC);
    (*pGC->ops->PutImage) (pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->width, pCursor->height,
 			   0, XYPixmap, pCursor->mask);

    /* mask bits -- pCursor->mask & ~pCursor->source */
    gcvals[0] = GXcopy;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC (pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) (pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->width, pCursor->height,
 			   0, XYPixmap, pCursor->mask);
    gcvals[0] = GXandInverted;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC (pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) (pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->width, pCursor->height,
 			   0, XYPixmap, pCursor->source);
    FreeScratchGC (pGC);
    return TRUE;
}

static Bool
miDCUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miDCCursorPtr   pPriv;

    pPriv = (miDCCursorPtr) pCursor->devPriv[pScreen->myNum];
    (*pScreen->DestroyPixmap) (pPriv->sourceBits);
    (*pScreen->DestroyPixmap) (pPriv->maskBits);
    xfree ((pointer) pPriv);
    return TRUE;
}

static void
miDCPutBits (pDrawable, pPriv, sourceGC, maskGC, x, y, w, h, source, mask)
    DrawablePtr	    pDrawable;
    GCPtr	    sourceGC, maskGC;
    miDCCursorPtr   pPriv;
    unsigned long   source, mask;
{
    XID	    gcvals[1];

    if (sourceGC->fgPixel != source)
    {
	gcvals[0] = source;
	DoChangeGC (sourceGC, GCForeground, gcvals, 0);
    }
    if (sourceGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, sourceGC);
    (*sourceGC->ops->PushPixels) (sourceGC, pPriv->sourceBits, pDrawable, w, h, x, y);
    if (maskGC->fgPixel != mask)
    {
	gcvals[0] = mask;
	DoChangeGC (maskGC, GCForeground, gcvals, 0);
    }
    if (maskGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, maskGC);
    (*maskGC->ops->PushPixels) (maskGC, pPriv->maskBits, pDrawable, w, h, x, y);
}

static Bool
miDCPutUpCursor (pScreen, pCursor, x, y, source, mask)
    ScreenPtr	    pScreen;
    CursorPtr	    pCursor;
    unsigned long   source, mask;
{
    miDCScreenPtr   pScreenPriv;
    miDCCursorPtr   pPriv;
    WindowPtr	    pWin;
    int		    status;
    XID		    gcvals[2];

    pPriv = (miDCCursorPtr) pCursor->devPriv[pScreen->myNum];
    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pWin = WindowTable[pScreen->myNum];
    if (!pScreenPriv->pSourceGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pSourceGC = CreateGC(pWin,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pSourceGC)
	    return FALSE;
	pScreenPriv->pMaskGC = CreateGC(pWin,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pMaskGC)
	{
	    FreeGC (pScreenPriv->pSourceGC, (GContext) 0);
	    pScreenPriv->pSourceGC = 0;
	    return FALSE;
	}
    }
    miDCPutBits (pWin, pPriv, pScreenPriv->pSourceGC, pScreenPriv->pMaskGC,
		 x, y, pCursor->width, pCursor->height, source, mask);
    return TRUE;
}

static Bool
miDCSaveUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    miDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    XID		    gcvals[2];
    int		    status;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pSave = pScreenPriv->pSave;
    pWin = WindowTable[pScreen->myNum];
    if (!pSave || pSave->drawable.width < w || pSave->drawable.height < h)
    {
	if (pSave)
	    (*pScreen->DestroyPixmap) (pSave);
	pScreenPriv->pSave = pSave =
		(*pScreen->CreatePixmap) (pScreen, w, h, pScreen->rootDepth);
	if (!pSave)
	    return FALSE;
    }
    if (!pScreenPriv->pSaveGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pSaveGC = CreateGC (pWin,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pSaveGC)
	    return FALSE;
    }
    pGC = pScreenPriv->pSaveGC;
    if (pSave->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pSave, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			    x, y, w, h, 0, 0);
    return TRUE;
}

static Bool
miDCRestoreUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    miDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    XID		    gcvals[2];
    int		    status;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pSave = pScreenPriv->pSave;
    pWin = WindowTable[pScreen->myNum];
    if (!pSave)
	return FALSE;
    if (!pScreenPriv->pRestoreGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pRestoreGC = CreateGC (pWin,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pRestoreGC)
	    return FALSE;
    }
    pGC = pScreenPriv->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}

static Bool
miDCMoveCursor (pScreen, pCursor, x, y, w, h, dx, dy, source, mask)
    ScreenPtr	    pScreen;
    CursorPtr	    pCursor;
    unsigned long   source, mask;
{
    miDCCursorPtr   pPriv;
    miDCScreenPtr   pScreenPriv;
    int		    status;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    XID		    gcvals[2];
    PixmapPtr	    pTemp;

    pPriv = (miDCCursorPtr) pCursor->devPriv[pScreen->myNum];
    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pWin = WindowTable[pScreen->myNum];
    pTemp = pScreenPriv->pTemp;
    if (!pTemp ||
	pTemp->drawable.width != pScreenPriv->pSave->drawable.width ||
	pTemp->drawable.height != pScreenPriv->pSave->drawable.height)
    {
	if (pTemp)
	    (*pScreen->DestroyPixmap) (pTemp);
	pScreenPriv->pTemp = pTemp = (*pScreen->CreatePixmap)
	    (pScreen, w, h, pScreenPriv->pSave->drawable.depth);
	if (!pTemp)
	    return FALSE;
    }
    if (!pScreenPriv->pMoveGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pMoveGC = CreateGC (pTemp,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pMoveGC)
	    return FALSE;
    }
    /*
     * copy the saved area to a temporary pixmap
     */
    pGC = pScreenPriv->pMoveGC;
    if (pGC->serialNumber != pTemp->drawable.serialNumber)
	ValidateGC ((DrawablePtr) pTemp, pGC);
    (*pGC->ops->CopyArea)
	(pScreenPriv->pSave, pTemp, pGC, 0, 0, w, h, 0, 0);
    
    /*
     * draw the cursor in the temporary pixmap
     */
    if (!pScreenPriv->pPixSourceGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pPixSourceGC = CreateGC (pTemp,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pPixSourceGC)
	    return FALSE;
    }
    if (!pScreenPriv->pPixMaskGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pPixMaskGC = CreateGC (pTemp,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pPixMaskGC)
	    return FALSE;
    }
    miDCPutBits (pTemp, pPriv, pScreenPriv->pPixSourceGC, pScreenPriv->pPixMaskGC,
 		 dx, dy, pCursor->width, pCursor->height, source, mask);

    /*
     * copy the temporary pixmap onto the screen
     */

    if (!pScreenPriv->pRestoreGC)
    {
	gcvals[0] = IncludeInferiors;
	gcvals[1] = FALSE;
	pScreenPriv->pRestoreGC = CreateGC (pWin,
	    GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
	if (!pScreenPriv->pRestoreGC)
	    return FALSE;
    }
    pGC = pScreenPriv->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);

    (*pGC->ops->CopyArea) ((DrawablePtr) pTemp, (DrawablePtr) pWin,
			    pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}
