/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: buffer.c,v 5.6 89/08/24 10:37:05 rws Exp $ */
#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _BUFFER_SERVER_	/* don't want Xlib structures */
#include "bufferstr.h"
#include "regionstr.h"
#include "gcstruct.h"

/*
 * per-buffer data
 */
 
typedef struct _Buffers	*BuffersPtr;

typedef struct _Buffer {
    BuffersPtr	pBuffers;	/* associated window data */
    Mask	eventMask;	/* ClobberNotifyMask|ExposureMask|UpdateNotifyMask */
    int		number;		/* index into array */
    int		side;		/* alwys Mono */
    PixmapPtr	pPixmap;	/* associated pixmap */
} BufferRec, *BufferPtr;

/*
 * per-window data
 */

typedef struct _Buffers {
    WindowPtr	pWindow;	/* associated window */
    int	    numBuffer;		/* count of buffers */
    int	    displayedBuffer;	/* currently active buffer */
    int	    updateAction;	/* Undefined, Background, Untouched, Copied */
    int	    updateHint;		/* Frequent, Intermittent, Static */
    int	    windowMode;		/* always Mono */

    unsigned short	width, height;	/* last known window size */
    short		x, y;		/* for static gravity */

    BufferPtr	buffers;
} BuffersRec;

typedef struct _BufferScreen {
    Bool    (*PositionWindow)();
} BufferScreenRec, *BufferScreenPtr;

static unsigned char	BufferReqCode;
static int		BufferEventBase;
static int		BufferErrorBase;
static int		BufferScreenIndex = -1;
static int		BufferWindowIndex = -1;

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
static RESTYPE	BufferDrawableResType;
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
static RESTYPE	BufferResType;
/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
static RESTYPE BuffersResType;

/*
 * make the resource id for buffer i refer to the window
 * drawable instead of the pixmap;
 */

AliasBuffer (pBuffers, i)
    BuffersPtr	pBuffers;
    int		i;
{
    BufferPtr	pBuffer;

    if (i == pBuffers->displayedBuffer)
	return;
    /*
     * remove the old association
     */
    if (pBuffers->displayedBuffer >= 0)
    {
	pBuffer = &pBuffers->buffers[pBuffers->displayedBuffer];
	ChangeResourceValue (pBuffer->pPixmap->drawable.id,
			     BufferDrawableResType,
 			     (pointer) pBuffer->pPixmap);
    }
    /*
     * make the new association
     */
    pBuffer = &pBuffers->buffers[i];
    ChangeResourceValue (pBuffer->pPixmap->drawable.id,
			 BufferDrawableResType,
			 (pointer) pBuffers->pWindow);
    pBuffers->displayedBuffer = i;
}

static
SetupBackgroundPainter (pWin, pGC)
    WindowPtr	pWin;
    GCPtr	pGC;
{
    XID		    gcvalues[4];
    int		    ts_x_origin, ts_y_origin;
    PixUnion	    background;
    int		    backgroundState;
    Mask	    gcmask;

    /*
     * set up the gc to clear the pixmaps;
     */
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
	gcvalues[2] = ts_x_origin;
	gcvalues[3] = ts_y_origin;
	gcmask = GCFillStyle|GCTile|GCTileStipXOrigin|GCTileStipYOrigin;
    }
    DoChangeGC(pGC, gcmask, gcvalues, TRUE);
}

/*
 * free everything associated with multibuffering for this
 * window
 */

DisposeBuffers (pWin)
    WindowPtr	pWin;
{
    FreeResourceByType (pWin->drawable.id, BuffersResType, FALSE);
}

/*
 * resize the buffers when the window is resized; this
 */ 

static Bool
BufferPositionWindow (pWin, x, y)
    WindowPtr	pWin;
    int		x, y;
{
    ScreenPtr	    pScreen;
    BufferScreenPtr pBufferScreen;
    BuffersPtr	    pBuffers;
    BufferPtr	    pBuffer;
    int		    width, height;
    int		    i;
    int		    gravity;
    int		    newx, newy;
    int		    dx, dy, dw, dh;
    int		    sourcex, sourcey;
    int		    destx, desty;
    PixmapPtr	    pPixmap;
    GCPtr	    pGC;
    int		    savewidth, saveheight;
    xRectangle	    clearRect;
    Bool	    clear;

    pScreen = pWin->drawable.pScreen;
    pBufferScreen = (BufferScreenPtr) pScreen->devPrivates[BufferScreenIndex].ptr;
    (*pBufferScreen->PositionWindow) (pWin, x, y);
    if (!(pBuffers = (BuffersPtr) pWin->devPrivates[BufferWindowIndex].ptr))
	return;
    if (pBuffers->width == pWin->drawable.width &&
        pBuffers->height == pWin->drawable.height)
	return;
    width = pWin->drawable.width;
    height = pWin->drawable.height;
    dx = pWin->drawable.x - pBuffers->x;
    dy = pWin->drawable.x - pBuffers->y;
    dw = width - pBuffers->width;
    dh = height - pBuffers->height;
    GravityTranslate (0, 0, -dx, -dy, dw, dh,
		      pWin->bitGravity, &destx, &desty);
    clear = pBuffers->width < width || pBuffers->height < height ||
	    pWin->bitGravity == ForgetGravity;

    sourcex = 0;
    sourcey = 0;
    savewidth = pBuffers->width;
    saveheight = pBuffers->height;
    /* clip rectangle to source and destination */
    if (destx < 0)
    {
	savewidth += destx;
	sourcex -= destx;
	destx = 0;
    }
    if (destx + savewidth > width)
	savewidth = width - destx;
    if (desty < 0)
    {
	saveheight += desty;
	sourcey -= desty;
	desty = 0;
    }
    if (desty + saveheight > height)
	saveheight = height - desty;

    pBuffers->width = width;
    pBuffers->height = height;
    pBuffers->x = pWin->drawable.x;
    pBuffers->y = pWin->drawable.y;

    pGC = GetScratchGC (pWin->drawable.depth, pScreen);
    if (clear)
    {
	SetupBackgroundPainter (pWin, pGC);
	clearRect.x = 0;
	clearRect.y = 0;
	clearRect.width = width;
	clearRect.height = height;
    }
    for (i = 0; i < pBuffers->numBuffer; i++)
    {
	pBuffer = &pBuffers->buffers[i];
	pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, pWin->drawable.depth);
	if (!pPixmap)
	{
	    DisposeBuffers (pWin);
	    break;
	}
	ValidateGC (pPixmap, pGC);
	/*
	 * I suppose this could avoid quite a bit of work if
	 * it computed the minimal area required.
	 */
	if (clear)
	    (*pGC->ops->PolyFillRect) (pPixmap, pGC, 1, &clearRect);
	if (pWin->bitGravity != ForgetGravity)
	{
	    (*pGC->ops->CopyArea) (pBuffer->pPixmap, pPixmap, pGC,
				    sourcex, sourcey, savewidth, saveheight,
				    destx, desty);
	}
	pPixmap->drawable.id = pBuffer->pPixmap->drawable.id;
	(*pScreen->DestroyPixmap) (pBuffer->pPixmap);
	pBuffer->pPixmap = pPixmap;
	if (i != pBuffers->displayedBuffer)
	{
	    ChangeResourceValue (pPixmap->drawable.id,
				 BufferDrawableResType,
				 (pointer) pPixmap);
	}
    }
    FreeScratchGC (pGC);
}

static void
BufferDrawableDelete (pDrawable, id)
    DrawablePtr	pDrawable;
    XID		id;
{
    WindowPtr	pWin;
    BuffersPtr	pBuffers;
    PixmapPtr	pPixmap;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDrawable;
	pBuffers = (BuffersPtr) pWin->devPrivates[BufferWindowIndex].ptr;
	pPixmap = pBuffers->buffers[pBuffers->displayedBuffer].pPixmap;
    }
    else
    {
	pPixmap = (PixmapPtr) pDrawable;
    }
    (*pPixmap->drawable.pScreen->DestroyPixmap) (pPixmap);
}

static void
BufferDelete (pBuffer, id)
    BufferPtr	pBuffer;
    XID		id;
{
    return;
}

static void
BuffersDelete (pBuffers, id)
    BuffersPtr	pBuffers;
    XID		id;
{
    int	i;

    for (i = 0; i < pBuffers->numBuffer; i++)
	FreeResource (pBuffers->buffers[i].pPixmap->drawable.id, 0);
    xfree (pBuffers->buffers);
    xfree (pBuffers);
}

/****************
 * BufferExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 ****************/

void
BufferExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();
    static int ProcBufferDispatch(), SProcBufferDispatch();
    static void  BufferResetProc();
    static void	SClobberNotifyEvent(), SUpdateNotifyEvent();
    static Bool	BufferPositionWindow();
    int	    i, j;
    ScreenPtr	pScreen;
    BufferScreenPtr pBufferScreen;

    BufferScreenIndex = AllocateScreenPrivateIndex ();
    if (BufferScreenIndex < 0)
	return;
    BufferWindowIndex = AllocateWindowPrivateIndex ();
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (!AllocateWindowPrivate (pScreen, BufferWindowIndex, 0) ||
	    !(pBufferScreen = (BufferScreenPtr) xalloc (sizeof (BufferScreenRec))))
	{
	    for (j = 0; j < i; j++)
		xfree (screenInfo.screens[j]->devPrivates[BufferScreenIndex].ptr);
	    return;
	}
	pScreen->devPrivates[BufferScreenIndex].ptr = (pointer) pBufferScreen;
	/*
 	 * wrap PositionWindow to resize the pixmap when the window
	 * changes size
 	 */
	pBufferScreen->PositionWindow = pScreen->PositionWindow;
	pScreen->PositionWindow = BufferPositionWindow;
    }
    /*
     * create the two resource types
     */
    BufferDrawableResType =
	CreateNewResourceType(BufferDrawableDelete)|RC_CACHED|RC_DRAWABLE;
    BufferResType = CreateNewResourceType(BufferDelete);
    BuffersResType = CreateNewResourceType(BuffersDelete);
    if (BufferDrawableResType && BufferResType && BuffersResType &&
	(extEntry = AddExtension(BUFFERNAME,
				 BufferNumberEvents, 
				 BufferNumberErrors,
				 ProcBufferDispatch, SProcBufferDispatch,
				 BufferResetProc, StandardMinorOpcode)))
    {
	BufferReqCode = (unsigned char)extEntry->base;
	BufferEventBase = extEntry->eventBase;
	BufferErrorBase = extEntry->errorBase;
	EventSwapVector[BufferEventBase + ClobberNotify] = SClobberNotifyEvent;
	EventSwapVector[BufferEventBase + UpdateNotify] = SUpdateNotifyEvent;
    }
}

/*ARGSUSED*/
static void
BufferResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int		    i;
    ScreenPtr	    pScreen;
    BufferScreenPtr pBufferScreen;
    
    if (BufferScreenIndex < 0)
	return;
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (pScreen->devPrivates[BufferScreenIndex].ptr)
	{
	    pBufferScreen = (BufferScreenPtr) pScreen->devPrivates[BufferScreenIndex].ptr;
	    pScreen->PositionWindow = pBufferScreen->PositionWindow;
	    xfree (pBufferScreen);
	}
    }
}

static int
ProcGetBufferVersion (client)
    register ClientPtr	client;
{
    REQUEST(xGetBufferVersionReq);
    xGetBufferVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xGetBufferVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = BUFFER_MAJOR_VERSION;
    rep.minorVersion = BUFFER_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xGetBufferVersionReply), (char *)&rep);
    return (client->noClientException);
}

ProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xCreateImageBuffersReq);
    xCreateImageBuffersReply	rep;
    register int		n;
    WindowPtr			pWin;
    XID				*ids;
    int				len, nbuf;
    int				i;
    BuffersPtr			pBuffers;
    BufferPtr			pBuffer;
    ScreenPtr			pScreen;
    int				width, height, depth;

    REQUEST_AT_LEAST_SIZE (xCreateImageBuffersReq);
    len = stuff->length - (sizeof(xCreateImageBuffersReq) >> 2);
    if (len == 0)
	return BadLength;
    if (!(pWin = LookupWindow (stuff->window, client)))
	return BadWindow;
    if (pWin->drawable.class == InputOnly)
	return BadMatch;
    switch (stuff->updateAction)
    {
    case UpdateActionUndefined:
    case UpdateActionBackground:
    case UpdateActionUntouched:
    case UpdateActionCopied:
	break;
    default:
	client->errorValue = stuff->updateAction;
	return BadValue;
    }
    switch (stuff->updateHint)
    {
    case UpdateHintFrequent:
    case UpdateHintIntermittent:
    case UpdateHintStatic:
	break;
    default:
	client->errorValue = stuff->updateHint;
	return BadValue;
    }
    nbuf = len;
    ids = (XID *) &stuff[1];
    for (i = 0; i < nbuf; i++)
    {
	LEGAL_NEW_RESOURCE(ids[i], client);
    }
    pBuffers = (BuffersPtr) xalloc (sizeof (BuffersRec));
    if (!pBuffers)
	return BadAlloc;
    pBuffers->pWindow = pWin;
    pBuffers->buffers = (BufferPtr) xalloc (nbuf * sizeof (BufferRec));
    if (!pBuffers->buffers)
    {
	xfree (pBuffers);
	return BadAlloc;
    }
    if (!AddResource (pWin->drawable.id, BuffersResType, (pointer) pBuffers))
    {
	xfree (pBuffers->buffers);
	xfree (pBuffers);
	return BadAlloc;
    }
    width = pWin->drawable.width;
    height = pWin->drawable.height;
    depth = pWin->drawable.depth;
    pScreen = pWin->drawable.pScreen;
    for (i = 0; i < nbuf; i++)
    {
	pBuffer = &pBuffers->buffers[i];
	pBuffer->eventMask = 0;
	pBuffer->number = i;
	pBuffer->side = BufferSideMono;
	pBuffer->pBuffers = pBuffers;
	if (!AddResource (ids[i], BufferResType, (pointer) pBuffer))
	    break;
	pBuffer->pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, depth);
	if (!pBuffer->pPixmap)
	    break;
	if (!AddResource (ids[i], BufferDrawableResType, (pointer) pBuffer->pPixmap))
	{
	    FreeResource (ids[i], BufferResType);
	    (*pScreen->DestroyPixmap) (pBuffer->pPixmap);
	    break;
	}
	pBuffer->pPixmap->drawable.id = ids[i];
    }
    pBuffers->numBuffer = i;
    pBuffers->displayedBuffer = -1;
    if (i > 0)
	AliasBuffer (pBuffers, 0);
    pBuffers->updateAction = stuff->updateAction;
    pBuffers->updateHint = stuff->updateHint;
    pBuffers->windowMode = WindowModeMono;
    pBuffers->width = width;
    pBuffers->height = height;
    pWin->devPrivates[BufferWindowIndex].ptr = (pointer) pBuffers;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.numberBuffer = pBuffers->numBuffer;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.numberBuffer, n);
    }
    WriteToClient(client, sizeof (xCreateImageBuffersReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xDisplayImageBuffersReq);
    BufferPtr	    *pBuffer;
    BuffersPtr	    *ppBuffers;
    WindowPtr	    pWin;
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    GCPtr	    pGC;
    PixmapPtr	    pPrevPixmap, pNewPixmap;
    xRectangle	    clearRect;
    
    REQUEST_AT_LEAST_SIZE (xDisplayImageBuffersReq);
    nbuf = stuff->length - (sizeof (xDisplayImageBuffersReq) >> 2);
    if (!nbuf)
	return Success;
    ids = (XID *) &stuff[1];
    ppBuffers = (BuffersPtr *) ALLOCATE_LOCAL (nbuf * sizeof (BuffersPtr));
    pBuffer = (BufferPtr *) ALLOCATE_LOCAL (nbuf * sizeof (BufferPtr));
    if (!ppBuffers || !pBuffer)
    {
	DEALLOCATE_LOCAL ((pointer) ppBuffers);
	DEALLOCATE_LOCAL ((pointer) pBuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    for (i = 0; i < nbuf; i++)
    {
	pBuffer[i] = (BufferPtr) LookupIDByType (ids[i], BufferResType);
	if (!pBuffer[i])
	{
	    DEALLOCATE_LOCAL ((pointer) ppBuffers);
	    DEALLOCATE_LOCAL ((pointer) pBuffer);
	    client->errorValue = ids[i];
	    return BufferErrorBase + BufferError;
	}
	ppBuffers[i] = pBuffer[i]->pBuffers;
	for (j = 0; j < i; j++)
	{
	    if (ppBuffers[i] == ppBuffers[j])
	    {
	    	DEALLOCATE_LOCAL ((pointer) ppBuffers);
	    	DEALLOCATE_LOCAL ((pointer) pBuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}
    }
    for (i = 0; i < nbuf; i++)
    {
	pWin = ppBuffers[i]->pWindow;
	pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
	pPrevPixmap = ppBuffers[i]->buffers[ppBuffers[i]->displayedBuffer].pPixmap;
	pNewPixmap = pBuffer[i]->pPixmap;
	switch (ppBuffers[i]->updateAction)
	{
	case UpdateActionUndefined:
	    break;
	case UpdateActionBackground:
	    SetupBackgroundPainter (pWin, pGC);
	    ValidateGC (pPrevPixmap, pGC);
	    clearRect.x = 0;
	    clearRect.y = 0;
	    clearRect.width = pPrevPixmap->drawable.width;
	    clearRect.height = pPrevPixmap->drawable.height;
	    (*pGC->ops->PolyFillRect) (pPrevPixmap, pGC, 1, &clearRect);
	    break;
	case UpdateActionUntouched:
	    ValidateGC (pPrevPixmap, pGC);
	    (*pGC->ops->CopyArea) ((DrawablePtr) pWin,
				   (DrawablePtr) pPrevPixmap,
 				   pGC,
				   0, 0,
 				   pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    break;
	case UpdateActionCopied:
	    ValidateGC (pPrevPixmap, pGC);
	    (*pGC->ops->CopyArea) (pNewPixmap, pPrevPixmap, pGC,
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    break;
	}
	ValidateGC (pWin, pGC);
	(*pGC->ops->CopyArea) (pNewPixmap, pWin, pGC,
			       0, 0, pWin->drawable.width, pWin->drawable.height,
			       0, 0);
	AliasBuffer (ppBuffers[i], pBuffer[i] - ppBuffers[i]->buffers);
	FreeScratchGC (pGC);
    }
    return Success;
}

static int
ProcDestroyImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST (xDestroyImageBuffersReq);
    WindowPtr	pWin;

    REQUEST_SIZE_MATCH (xDestroyImageBuffersReq);
    if (!(pWin = LookupWindow (stuff->window)))
	return BadWindow;
    DisposeBuffers (pWin);
    return Success;
}

static int
ProcBufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_GetBufferVersion:
	return ProcGetBufferVersion (client);
    case X_CreateImageBuffers:
	return ProcCreateImageBuffers (client);
    case X_DisplayImageBuffers:
	return ProcDisplayImageBuffers (client);
    case X_DestroyImageBuffers:
	return ProcDestroyImageBuffers (client);
    default:
	return BadRequest;
    }
}

static int
SProcGetBufferVersion (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xGetBufferVersionReq);

    swaps (&stuff->length, n);
    return ProcGetBufferVersion (client);
}

static int
SProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xCreateImageBuffersReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xCreateImageBuffersReq);
    swapl (&stuff->window, n);
    SwapRestL(stuff);
    return ProcCreateImageBuffers (client);
}

static int
SProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xDisplayImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xDisplayImageBuffersReq);
    swaps (&stuff->minDelay, n);
    swaps (&stuff->maxDelay, n);
    SwapRestL(stuff);
    return ProcDisplayImageBuffers (client);
}

static int
SProcDestroyImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xDestroyImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xDestroyImageBuffersReq);
    swapl (&stuff->window, n);
    return ProcDestroyImageBuffers (client);
}

static int
SProcBufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_GetBufferVersion:
	return SProcGetBufferVersion (client);
    case X_CreateImageBuffers:
	return SProcCreateImageBuffers (client);
    case X_DisplayImageBuffers:
	return SProcDisplayImageBuffers (client);
    case X_DestroyImageBuffers:
	return SProcDestroyImageBuffers (client);
    default:
	return BadRequest;
    }
}

static void
SUpdateNotifyEvent ()
{
}

static void
SClobberNotifyEvent ()
{
}
