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

/* $XConsortium: multibuf.c,v 1.11 91/06/01 13:25:50 rws Exp $ */
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
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"
#include <sys/time.h>

#define _MULTIBUF_SERVER_	/* don't want Xlib structures */
#include "multibufst.h"


#define SameClient(obj,client) \
	(CLIENT_BITS((obj)->resource) == (client)->clientAsMask)
#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])
#define bClient(b)   (clients[CLIENT_ID(b->pDrawable->id)])

#define ValidEventMasks (ExposureMask|MultibufferClobberNotifyMask|MultibufferUpdateNotifyMask)


/*
 * per display-image-buffers request data.
 */

typedef struct _DisplayRequest {
    struct _DisplayRequest	*next;
    TimeStamp			activateTime;
    ClientPtr			pClient;
    XID				id;
} DisplayRequestRec, *DisplayRequestPtr;

static unsigned char	MultibufferReqCode;
static int		MultibufferEventBase;
static int		MultibufferErrorBase;

int		        MultibufferScreenIndex = -1;
int		        MultibufferWindowIndex = -1;

static int		BlockHandlerRegistered;
static void		MultibufferBlockHandler(), MultibufferWakeupHandler();

static void		PerformDisplayRequest ();
static void		DisposeDisplayRequest ();
static Bool		QueueDisplayRequest ();

static void		BumpTimeStamp ();

static void		AliasMultibuffer ();
static void		RecalculateMultibufferOtherEvents ();
static int		EventSelectForMultibuffer();
static Bool		MultibuffersDestroyWindow();

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
RESTYPE			MultibufferDrawableResType;
static void		MultibufferDrawableDelete ();
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
static RESTYPE		MultibufferResType;
static void		MultibufferDelete ();
/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
static void		MultibuffersDelete ();
/*
 * Per display-buffers request is attached to a resource so that
 * it will disappear if the client dies before the request should
 * be processed
 */
static RESTYPE		DisplayRequestResType;
static void		DisplayRequestDelete ();
/*
 * Clients other than the buffer creator attach event masks in
 * OtherClient structures; each has a resource of this type.
 */
static RESTYPE		OtherClientResType;
static void		OtherClientDelete ();



/****************
 * RegisterMultibufferInit
 *
 * Should be called per-screen to register function to initialize
 * MultibufferScreenRec. If not, the initializer is defaulted to
 * pixMultibufferInit(), which is for the multibuffer pixmap version.
 *
 ****************/

static mbInitFunc mbInitFuncs[MAXSCREENS];

void RegisterMultibufferInit(pScreen, initFunc)
    ScreenPtr	pScreen;
    mbInitFunc	initFunc;
{
    mbInitFuncs[pScreen->myNum] = initFunc;
}

/****************
 * MultibufferExtensionInit
 *
 * Called from InitExtensions in main()
 *
 ****************/

static int		ProcMultibufferDispatch(), SProcMultibufferDispatch();
static void		MultibufferResetProc();
static void		SClobberNotifyEvent(), SUpdateNotifyEvent();
static Bool		MultibufferPositionWindow();

extern Bool		    pixMultibufferInit(); /* Default initializer */

void
MultibufferExtensionInit()
{
    ExtensionEntry	    *extEntry;
    int			    i, j;
    ScreenPtr		    pScreen;
    mbufScreenPtr    pMBScreen;

    /*
     * allocate private pointers in windows and screens.  Allocating
     * window privates may seem like an unnecessary expense, but every
     * PositionWindow call must check to see if the window is
     * multi-buffered; a resource lookup is too expensive.
     */
    MultibufferScreenIndex = AllocateScreenPrivateIndex ();
    if (MultibufferScreenIndex < 0)
	return;
    MultibufferWindowIndex = AllocateWindowPrivateIndex ();
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (!AllocateWindowPrivate (pScreen, MultibufferWindowIndex, 0) ||
	    !(pMBScreen = (mbufScreenPtr) xalloc (sizeof (mbufScreenRec))))
	{
	    for (j = 0; j < i; j++)
		xfree (MB_SCREEN_PRIV(screenInfo.screens[j]));
	    return;
	}
	pScreen->devPrivates[MultibufferScreenIndex].ptr = (pointer) pMBScreen;

	/* If not registered, give them the multibuffer pixmap code */

	if (! mbInitFuncs[i])
	    mbInitFuncs[i] = pixMultibufferInit;

	if (! (* mbInitFuncs[i])(pScreen,pMBScreen))
	{
	    for (j = 0; j < i; j++)
		xfree (MB_SCREEN_PRIV(screenInfo.screens[j]));
	    return;
	}

	pMBScreen->mbufWindowCount = 0;
	pMBScreen->DestroyWindow   = NULL;
	pMBScreen->funcsWrapped    = 0;
    }
    /*
     * create the resource types
     */
    MultibufferDrawableResType =
	CreateNewResourceType(MultibufferDrawableDelete)|RC_CACHED|RC_DRAWABLE;
    MultibufferResType = CreateNewResourceType(MultibufferDelete);
    DisplayRequestResType = CreateNewResourceType(DisplayRequestDelete);
    OtherClientResType = CreateNewResourceType(OtherClientDelete);
    if (MultibufferDrawableResType && MultibufferResType &&
	DisplayRequestResType && OtherClientResType &&
	(extEntry = AddExtension(MULTIBUFFER_PROTOCOL_NAME,
				 MultibufferNumberEvents, 
				 MultibufferNumberErrors,
				 ProcMultibufferDispatch, SProcMultibufferDispatch,
				 MultibufferResetProc, StandardMinorOpcode)))
    {
	MultibufferReqCode = (unsigned char)extEntry->base;
	MultibufferEventBase = extEntry->eventBase;
	MultibufferErrorBase = extEntry->errorBase;
	EventSwapVector[MultibufferEventBase + MultibufferClobberNotify] = SClobberNotifyEvent;
	EventSwapVector[MultibufferEventBase + MultibufferUpdateNotify] = SUpdateNotifyEvent;
    }
}

/*ARGSUSED*/
static void
MultibufferResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int			    i;
    ScreenPtr		    pScreen;
    mbufScreenPtr    pMBScreen;
    
    if (MultibufferScreenIndex < 0)
	return;
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (pMBScreen = MB_SCREEN_PRIV(pScreen))
	{
	    (* pMBScreen->ResetProc)(pScreen);
	    xfree (pMBScreen);
	}
    }
}

static int
ProcGetBufferVersion (client)
    register ClientPtr	client;
{
    REQUEST(xMbufGetBufferVersionReq);
    xMbufGetBufferVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xMbufGetBufferVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = MULTIBUFFER_MAJOR_VERSION;
    rep.minorVersion = MULTIBUFFER_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xMbufGetBufferVersionReply), (char *)&rep);
    return (client->noClientException);
}

int
CreateImageBuffers (pWin, nbuf, ids, action, hint)
    WindowPtr	pWin;
    int		nbuf;
    XID		*ids;
    int		action;
    int		hint;
{
    ScreenPtr		pScreen;
    mbufScreenPtr	pMBScreen;
    mbufWindowPtr	pMBWindow;
    mbufBufferPtr	pMBBuffer;
    int			i;

    VisualID		visualID;
    int			depth;
    xMbufBufferInfo	*pInfo;


    pScreen   = pWin->drawable.pScreen;
    pMBScreen = MB_SCREEN_PRIV(pScreen);

    /*
     * See if multibuffering is supported for the windows visual and depth.
     * If so, truncate the number of buffers requested by maxBuffers.
     */

    visualID = wVisual(pWin);
    depth    = pWin->drawable.depth;
    for (i=pMBScreen->nInfo, pInfo=pMBScreen->pInfo; i--; pInfo++)
    {
	if ((pInfo->visualID == visualID) && (pInfo->depth == depth))
	{
	    /* maxBuffers of 0 indicates that there is no fixed limit */
	    if (pInfo->maxBuffers && (nbuf > pInfo->maxBuffers))
		nbuf = pInfo->maxBuffers;
	    break;
	}
    }

    if (i==0)
	return (BadMatch); /* Window's depth or visual not supported. */

    /* 
     * In the description of the CreateImageBuffers request:
     * "If the window already has a group of image buffers
     *  associated with it (ie: from a previous
     *  CreateImageBuffers request) the actions described
     *  for DestroyImageBuffers are performed first (this
     *  will delete the association of the previous buffer
     *  ID's and their buffers as well as deallocate all
     *  buffers except for the one already associated with
     *  the window ID)."
     */

    if (MB_WINDOW_PRIV(pWin))
        DestroyImageBuffers(pWin);

    /* build the window private */

    pMBWindow = (mbufWindowPtr) xalloc (sizeof (mbufWindowRec));
    if (!pMBWindow)
	return BadAlloc;
    pMBWindow->buffers = (mbufBufferPtr) xalloc (nbuf * sizeof(mbufBufferRec));
    if (!pMBWindow->buffers)
    {
	xfree (pMBWindow);
	return BadAlloc;
    }

    pMBWindow->pWindow = pWin;
    pMBWindow->displayedMultibuffer = -1; /* Needs to be -1 */
    pMBWindow->updateAction = action;
    pMBWindow->updateHint = hint;
    pMBWindow->windowMode = MultibufferModeMono;
    pMBWindow->lastUpdate.months = 0;
    pMBWindow->lastUpdate.milliseconds = 0;
    pMBWindow->width = pWin->drawable.width;
    pMBWindow->height = pWin->drawable.height;
    pWin->devPrivates[MultibufferWindowIndex].ptr = (pointer) pMBWindow;

    for (i = 0; i < nbuf; i++)
    {
	pMBBuffer = pMBWindow->buffers + i;

	pMBBuffer->pMBWindow = pMBWindow;
	pMBBuffer->eventMask = 0L;
	pMBBuffer->otherEventMask = 0L;
	pMBBuffer->otherClients = (OtherClientsPtr) NULL;
	pMBBuffer->number = i;
	pMBBuffer->side = MultibufferSideMono;
	pMBBuffer->clobber = MultibufferUnclobbered;
	pMBBuffer->pDrawable = NULL;

	if (!AddResource (ids[i], MultibufferResType, (pointer) pMBBuffer))
	{
	    nbuf = i;
	    break;
	}
    }

    /* Call hook for ddx. Returns number of buffers it can support. */
    pMBWindow->numMultibuffer =
	(* pMBScreen->CreateImageBuffers) (pWin, nbuf, ids, action, hint);

    /* Free extra ids if the number of buffers returned is smaller */
    for (i=pMBWindow->numMultibuffer; i < nbuf; i++)
	FreeResource (ids[i], MultibufferResType);

    if (pMBWindow->numMultibuffer == 0)
    {
	xfree (pMBWindow->buffers);
	xfree (pMBWindow);
	pWin->devPrivates[MultibufferWindowIndex].ptr = NULL;
	return BadAlloc;
    }

    AliasMultibuffer (pMBWindow, 0);

    /* Wrap screen funcs if this is the first multibuffered window */
    if (pMBScreen->mbufWindowCount == 0)
    {
	(* pMBScreen->WrapScreenFuncs)(pScreen);
	WRAP_SCREEN_FUNC(pScreen, pMBScreen,
			 DestroyWindow, MultibuffersDestroyWindow);
    }
    pMBScreen->mbufWindowCount++;
    return Success;
}

static int
ProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xMbufCreateImageBuffersReq);
    xMbufCreateImageBuffersReply	rep;
    register int		n;
    WindowPtr			pWin;
    XID				*ids;
    int				len, nbuf;
    int				i;
    int				err;

    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    len = stuff->length - (sizeof(xMbufCreateImageBuffersReq) >> 2);
    if (len == 0)
	return BadLength;
    if (!(pWin = LookupWindow (stuff->window, client)))
	return BadWindow;
    if (pWin->drawable.class == InputOnly)
	return BadMatch;
    switch (stuff->updateAction)
    {
    case MultibufferUpdateActionUndefined:
    case MultibufferUpdateActionBackground:
    case MultibufferUpdateActionUntouched:
    case MultibufferUpdateActionCopied:
	break;
    default:
	client->errorValue = stuff->updateAction;
	return BadValue;
    }
    switch (stuff->updateHint)
    {
    case MultibufferUpdateHintFrequent:
    case MultibufferUpdateHintIntermittent:
    case MultibufferUpdateHintStatic:
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
    err = CreateImageBuffers (pWin, nbuf, ids,
			      stuff->updateAction, stuff->updateHint);
    if (err != Success)
	return err;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.numberBuffer = MB_WINDOW_PRIV(pWin)->numMultibuffer;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.numberBuffer, n);
    }
    WriteToClient(client, sizeof (xMbufCreateImageBuffersReply), (char *)&rep);
    return (client->noClientException);
}



static int
ProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xMbufDisplayImageBuffersReq);
    mbufBufferPtr	    *pMBBuffer;
    mbufWindowPtr	    *ppMBWindow;
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    CARD32	    minDelay, maxDelay;
    TimeStamp	    activateTime, bufferTime;
    
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    nbuf = stuff->length - (sizeof (xMbufDisplayImageBuffersReq) >> 2);
    if (!nbuf)
	return (client->noClientException);
    minDelay = stuff->minDelay;
    maxDelay = stuff->maxDelay;
    ids = (XID *) &stuff[1];
    ppMBWindow = (mbufWindowPtr *) ALLOCATE_LOCAL (nbuf*sizeof(mbufWindowPtr));
    pMBBuffer = (mbufBufferPtr *) ALLOCATE_LOCAL (nbuf*sizeof(mbufBufferPtr));
    if (!ppMBWindow || !pMBBuffer)
    {
	DEALLOCATE_LOCAL (ppMBWindow);
	DEALLOCATE_LOCAL (pMBBuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    activateTime.months = 0;
    activateTime.milliseconds = 0;
    for (i = 0; i < nbuf; i++)
    {
	pMBBuffer[i] = (mbufBufferPtr) LookupIDByType(ids[i],MultibufferResType);
	if (!pMBBuffer[i])
	{
	    DEALLOCATE_LOCAL (ppMBWindow);
	    DEALLOCATE_LOCAL (pMBBuffer);
	    client->errorValue = ids[i];
	    return MultibufferErrorBase + MultibufferBadBuffer;
	}
	ppMBWindow[i] = pMBBuffer[i]->pMBWindow;

        /* Enforce the following statement from the protocol:
         * "Attempting to  simultaneously  display  multiple image
         *  buffers from the same window is an error (Match) since it
         *  violates  the  rule that  only  one image buffer per group
         *  can be displayed at a time."
         */

	for (j = 0; j < i; j++)
	{
	    if (ppMBWindow[i] == ppMBWindow[j])
	    {
	    	DEALLOCATE_LOCAL (ppMBWindow);
	    	DEALLOCATE_LOCAL (pMBBuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}

        /* "For each of the windows to be updated by this
         *  request, at least min delay milli-seconds should
         *  elapse since the last time any of the windows were
         *  updated..."
         */

	bufferTime = ppMBWindow[i]->lastUpdate;
	BumpTimeStamp (&bufferTime, minDelay);
	if (CompareTimeStamps (bufferTime, activateTime) == LATER)
	    activateTime = bufferTime;
    }

    UpdateCurrentTime ();
    if (CompareTimeStamps (activateTime, currentTime) == LATER &&
	QueueDisplayRequest (client, activateTime))
    {
	;
    }
    else
	PerformDisplayRequest (ppMBWindow, pMBBuffer, nbuf);

    DEALLOCATE_LOCAL (ppMBWindow);
    DEALLOCATE_LOCAL (pMBBuffer);
    return Success;
}

static int
ProcDestroyImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST (xMbufDestroyImageBuffersReq);
    WindowPtr	pWin;

    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    if (!(pWin = LookupWindow (stuff->window, client)))
	return BadWindow;
    DestroyImageBuffers(pWin);
    return (client->noClientException);
}

static int
ProcSetMBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST (xMbufSetMBufferAttributesReq);
    WindowPtr	pWin;
    mbufWindowPtr	pMBWindow;
    int		len;
    Mask	vmask;
    Mask	index;
    CARD32	updateHint;
    XID		*vlist;

    REQUEST_AT_LEAST_SIZE (xMbufSetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    if (!(pMBWindow = MB_WINDOW_PRIV(pWin)))
	return BadMatch;
    len = stuff->length - (sizeof (xMbufSetMBufferAttributesReq) >> 2);
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    vlist = (XID *) &stuff[1];
    while (vmask)
    {
	index = (Mask) lowbit (vmask);
	vmask &= ~index;
	switch (index)
	{
	case MultibufferWindowUpdateHint:
	    updateHint = (CARD32) *vlist;
	    switch (updateHint)
	    {
	    case MultibufferUpdateHintFrequent:
	    case MultibufferUpdateHintIntermittent:
	    case MultibufferUpdateHintStatic:
		pMBWindow->updateHint = updateHint;
		break;
	    default:
		client->errorValue = updateHint;
		return BadValue;
	    }
	    vlist++;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
	    return BadValue;
	}
    }

    /* XXX - Should check return code */
    (* MB_SCREEN_PRIV(pWin->drawable.pScreen)->ChangeMBufferAttributes)
	(pMBWindow, stuff->valueMask);
    return (client->noClientException);
}

static int
ProcGetMBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xMbufGetMBufferAttributesReq);
    WindowPtr	pWin;
    mbufWindowPtr	pMBWindow;
    XID		*ids;
    xMbufGetMBufferAttributesReply  rep;
    int		i, n;

    REQUEST_SIZE_MATCH (xMbufGetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    if (!(pMBWindow=MB_WINDOW_PRIV(pWin)))
	return BadAccess;
    ids = (XID *) ALLOCATE_LOCAL (pMBWindow->numMultibuffer * sizeof (XID));
    if (!ids)
	return BadAlloc;
    for (i = 0; i < pMBWindow->numMultibuffer; i++)
	ids[i] = pMBWindow->buffers[i].pDrawable->id;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = pMBWindow->numMultibuffer;
    rep.displayedBuffer = pMBWindow->displayedMultibuffer;
    rep.updateAction = pMBWindow->updateAction;
    rep.updateHint = pMBWindow->updateHint;
    rep.windowMode = pMBWindow->windowMode;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.displayedBuffer, n);
	SwapLongs (ids, pMBWindow->numMultibuffer);
    }
    WriteToClient (client, sizeof (xMbufGetMBufferAttributesReply), &rep);
    WriteToClient (client, (int) (pMBWindow->numMultibuffer * sizeof (XID)), ids);
    DEALLOCATE_LOCAL((pointer) ids);
    return client->noClientException;
}

static int
ProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xMbufSetBufferAttributesReq);
    mbufBufferPtr	pMBBuffer;
    int		len;
    Mask	vmask, index;
    XID		*vlist;
    Mask	eventMask;
    int		result;

    REQUEST_AT_LEAST_SIZE (xMbufSetBufferAttributesReq);
    pMBBuffer = (mbufBufferPtr) LookupIDByType (stuff->buffer, MultibufferResType);
    if (!pMBBuffer)
	return MultibufferErrorBase + MultibufferBadBuffer;
    len = stuff->length - (sizeof (xMbufSetBufferAttributesReq) >> 2);
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    vlist = (XID *) &stuff[1];
    while (vmask)
    {
	index = (Mask) lowbit (vmask);
	vmask &= ~index;
	switch (index)
	{
	case MultibufferBufferEventMask:
	    eventMask = (Mask) *vlist;
	    vlist++;
	    result = EventSelectForMultibuffer (pMBBuffer, client, eventMask);
	    if (result != Success)
		return result;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
	    return BadValue;
	}
    }

    /* XXX - Should check return code */
    (* MB_SCREEN_PRIV(pMBBuffer->pDrawable->pScreen)->ChangeBufferAttributes)
	(pMBBuffer, stuff->valueMask);

    return (client->noClientException);
}

ProcGetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xMbufGetBufferAttributesReq);
    mbufBufferPtr	pMBBuffer;
    xMbufGetBufferAttributesReply	rep;
    OtherClientsPtr		other;
    int				n;

    REQUEST_SIZE_MATCH (xMbufGetBufferAttributesReq);
    pMBBuffer = (mbufBufferPtr) LookupIDByType(stuff->buffer,MultibufferResType);
    if (!pMBBuffer)
	return MultibufferErrorBase + MultibufferBadBuffer;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.window = pMBBuffer->pMBWindow->pWindow->drawable.id;
    if (bClient (pMBBuffer) == client)
	rep.eventMask = pMBBuffer->eventMask;
    else
    {
	rep.eventMask = (Mask) 0L;
	for (other = pMBBuffer->otherClients; other; other = other->next)
	    if (SameClient (other, client))
	    {
		rep.eventMask = other->mask;
		break;
	    }
    }
    rep.bufferIndex = pMBBuffer->number;
    rep.side = pMBBuffer->side;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.window, n);
	swapl(&rep.eventMask, n);
	swaps(&rep.bufferIndex, n);
    }
    WriteToClient(client, sizeof(xMbufGetBufferAttributesReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcGetBufferInfo (client)
    register ClientPtr	client;
{
    REQUEST (xMbufGetBufferInfoReq);
    DrawablePtr		    pDrawable;
    xMbufGetBufferInfoReply rep;
    ScreenPtr		    pScreen;
    int			    i;
    int			    n;
    xMbufBufferInfo	    *pInfo;
    int			    nInfo;
    pDrawable = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDrawable)
	return BadDrawable;

    pScreen = pDrawable->pScreen;
    nInfo = MB_SCREEN_PRIV(pScreen)->nInfo;
    pInfo = MB_SCREEN_PRIV(pScreen)->pInfo;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = nInfo * (sizeof (xMbufBufferInfo) >> 2);
    rep.normalInfo = nInfo;
    rep.stereoInfo = 0;

    if (client->swapped)
    {
	xMbufBufferInfo	    *pInfoOrig = pInfo;

	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.normalInfo, n);
	swaps(&rep.stereoInfo, n);

	pInfo = (xMbufBufferInfo *)
	    ALLOCATE_LOCAL(nInfo * sizeof (xMbufBufferInfo));
	if (!pInfo)
	    return BadAlloc;

	for (i = 0; i < nInfo; i++)
	{
	    cpswapl (pInfoOrig[i].visualID, pInfo[i].visualID);
	    cpswaps (pInfoOrig[i].maxBuffers, pInfo[i].maxBuffers);
	    pInfoOrig[i].depth = pInfo[i].depth;
	}
    }

    WriteToClient (client, sizeof(xMbufGetBufferInfoReply), (pointer) &rep);
    WriteToClient (client, nInfo * sizeof(xMbufBufferInfo), (pointer) pInfo);

    if (client->swapped)
	DEALLOCATE_LOCAL(pInfo);
    return client->noClientException;
}

static int
ProcClearImageBufferArea (client)
    register ClientPtr	client;
{
    REQUEST (xMbufClearImageBufferAreaReq);
    mbufBufferPtr	pMBBuffer;
    ScreenPtr		pScreen;

    REQUEST_SIZE_MATCH (xMbufClearImageBufferAreaReq);

    pMBBuffer = (mbufBufferPtr)
	LookupIDByType(stuff->buffer, MultibufferResType);

    if (!pMBBuffer || !pMBBuffer->pDrawable)
	return MultibufferErrorBase + MultibufferBadBuffer;

    if ((stuff->exposures != xTrue) && (stuff->exposures != xFalse))
    {
	client->errorValue = stuff->exposures;
        return(BadValue);
    }

    pScreen = pMBBuffer->pDrawable->pScreen;
    (* MB_SCREEN_PRIV(pScreen)->ClearImageBufferArea) (pMBBuffer,
				stuff->x, stuff->y,
				stuff->width, stuff->height,
				(Bool) stuff->exposures);
    
    return (client->noClientException);
}

static int
ProcMultibufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return ProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return ProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return ProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return ProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return ProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return ProcGetMBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return ProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return ProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return ProcGetBufferInfo (client);
    case X_MbufClearImageBufferArea:
        return ProcClearImageBufferArea (client);
    default:
	return BadRequest;
    }
}

static int
SProcGetBufferVersion (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferVersionReq);

    swaps (&stuff->length, n);
    return ProcGetBufferVersion (client);
}

static int
SProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufCreateImageBuffersReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    swapl (&stuff->window, n);
    SwapRestL(stuff);
    return ProcCreateImageBuffers (client);
}

static int
SProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufDisplayImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
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
    REQUEST (xMbufDestroyImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    swapl (&stuff->window, n);
    return ProcDestroyImageBuffers (client);
}

static int
SProcSetMBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufSetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetMBufferAttributesReq);
    swapl (&stuff->window, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetMBufferAttributes (client);
}

static int
SProcGetMBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetMBufferAttributesReq);
    swapl (&stuff->window, n);
    return ProcGetMBufferAttributes (client);
}

static int
SProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufSetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetBufferAttributes (client);
}

static int
SProcGetBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    return ProcGetBufferAttributes (client);
}

static int
SProcGetBufferInfo (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferInfoReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufGetBufferInfoReq);
    swapl (&stuff->drawable, n);
    return ProcGetBufferInfo (client);
}

static int
SProcClearImageBufferArea(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xMbufClearImageBufferAreaReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufClearImageBufferAreaReq);
    swapl(&stuff->buffer, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return ProcClearImageBufferArea(client);
}

static int
SProcMultibufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return SProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return SProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return SProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return SProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return SProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return SProcGetMBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return SProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return SProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return SProcGetBufferInfo (client);
    case X_MbufClearImageBufferArea:
	return SProcClearImageBufferArea (client);
    default:
	return BadRequest;
    }
}

static void
SUpdateNotifyEvent (from, to)
    xMbufUpdateNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    cpswapl (from->timeStamp, to->timeStamp);
}

static void
SClobberNotifyEvent (from, to)
    xMbufClobberNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    to->state = from->state;
}

/* Compare function to pass to qsort */
static int
BufferCompareByScreen(pMBBuffer1, pMBBuffer2)
    mbufBufferPtr	    *pMBBuffer1;
    mbufBufferPtr	    *pMBBuffer2;
{
    int n1, n2;

    n1 = (* pMBBuffer1)->pMBWindow->pWindow->drawable.pScreen->myNum;
    n2 = (* pMBBuffer2)->pMBWindow->pWindow->drawable.pScreen->myNum;
    if (n1 == n2)
	return 0;
    return ((n1 > n2) ? 1 : -1);
}

static void
PerformDisplayRequest (ppMBWindow, ppMBBuffer, nbuf)
    mbufBufferPtr	    *ppMBBuffer;
    mbufWindowPtr	    *ppMBWindow;
    int		    nbuf;
{
    ScreenPtr            pScreen, currentScreen;
    int		         i, currentIndex;

    UpdateCurrentTime ();

    currentIndex  = 0;
    currentScreen = ppMBWindow[0]->pWindow->drawable.pScreen;

    /*
     * Buffers can be from different screens. Group buffers by their
     * screen and call the per-screen DisplayImageBuffers for each group.
     */

    if ((nbuf > 1) && (screenInfo.numScreens > 1))
    {
	/* Sort buffers by screen number */
	qsort(ppMBBuffer, nbuf, sizeof(* ppMBBuffer), BufferCompareByScreen);
	for (i = 0; i < nbuf; i++)
	    ppMBWindow[i] = ppMBBuffer[i]->pMBWindow;
	
	/* Call DisplayImageBuffer for each different screen (except last) */
	currentScreen = ppMBWindow[0]->pWindow->drawable.pScreen;
	for (i=1; i<nbuf; i++)
	{
	    pScreen   = ppMBWindow[i]->pWindow->drawable.pScreen;
	    
	    if (pScreen->myNum != currentScreen->myNum)
	    {
		(* MB_SCREEN_PRIV(currentScreen)->DisplayImageBuffers)
		    (currentScreen, &ppMBWindow[currentIndex],
		     &ppMBBuffer[currentIndex], i - currentIndex);
		currentIndex = i;
		currentScreen = pScreen;
	    }
	}
    }

    (* MB_SCREEN_PRIV(currentScreen)->DisplayImageBuffers)
	(currentScreen, &ppMBWindow[currentIndex],
	 &ppMBBuffer[currentIndex], nbuf - currentIndex);

    for (i = 0; i < nbuf; i++)
    {
	ppMBWindow[i]->lastUpdate = currentTime;

	/* Send UpdateNotify on previously displayed buffer.
	 * MB_DISPLAYED_BUFFER(ppWindow[i]) does not get modified
	 * until AliasMultbuffer() so it still has the number of
	 * the previously displayed buffer.
	 */
	MultibufferUpdate (MB_DISPLAYED_BUFFER(ppMBWindow[i]),
			   ppMBWindow[i]->lastUpdate.milliseconds);
	AliasMultibuffer (ppMBWindow[i], ppMBBuffer[i]->number);
    }
}

DrawablePtr
GetBufferPointer (pWin, i)
    WindowPtr	pWin;
    int		i;
{
    mbufWindowPtr pMBWindow;

    if (!(pMBWindow = MB_WINDOW_PRIV(pWin)))
	return NULL;
    return pMBWindow->buffers[i].pDrawable;
}

/*
 * Internal interface for multibuffer users residing in server, such as PEX.
 * Executes request immediately (does not queue).
 */

int
DisplayImageBuffers (ids, nbuf)
    XID	    *ids;
    int	    nbuf;
{
    mbufBufferPtr *ppMBBuffer;
    mbufWindowPtr *ppMBWindow;
    int		  i, j;

    ppMBBuffer = (mbufBufferPtr *) ALLOCATE_LOCAL (nbuf * sizeof(*ppMBBuffer) +
				   nbuf * sizeof(*ppMBWindow));
    if (!ppMBBuffer)
	return BadAlloc;
    ppMBWindow = (mbufWindowPtr *) (ppMBBuffer + nbuf);
    for (i = 0; i < nbuf; i++)
    {
	ppMBBuffer[i] = (mbufBufferPtr) LookupIDByType (ids[i], MultibufferResType);
	if (!ppMBBuffer[i])
	{
	    DEALLOCATE_LOCAL (ppMBBuffer);
	    return MultibufferErrorBase + MultibufferBadBuffer;
	}
	ppMBWindow[i] = ppMBBuffer[i]->pMBWindow;
	for (j = 0; j < i; j++)
	    if (ppMBWindow[i] == ppMBWindow[j])
	    {
		DEALLOCATE_LOCAL (ppMBBuffer);
		return BadMatch;
	    }
    }
    PerformDisplayRequest (ppMBWindow, ppMBBuffer, nbuf);
    DEALLOCATE_LOCAL (ppMBBuffer);
    return Success;
}

/* A display request waits on this simple linked list until it is time
 * to be executed.  The display requests are ordered by increasing TimeStamp
 * (earliest to latest).
 */
static DisplayRequestPtr    pPendingRequests = NULL;

static void
DisposeDisplayRequest (pRequest)
    DisplayRequestPtr	pRequest;
{
    DisplayRequestPtr	pReq, pPrev;

    pPrev = NULL;
    for (pReq = pPendingRequests; pReq;	pReq = pReq->next)
    {
	if (pReq == pRequest)
	{
	    if (pPrev)
		pPrev->next = pReq->next;
	    else
		pPendingRequests = pReq->next;
	    xfree (pReq);
	    break;
	}
	pPrev = pReq;
    }
}

/* insert a new display request onto pPendingRequests, return TRUE if
 * successful
 */
static Bool
QueueDisplayRequest (client, activateTime)
    ClientPtr	    client;
    TimeStamp	    activateTime;
{
    DisplayRequestPtr	pRequest, pReq, pPrev;

    if (!BlockHandlerRegistered)
    {
	if (!RegisterBlockAndWakeupHandlers (MultibufferBlockHandler,
					     MultibufferWakeupHandler,
					     (pointer) 0))
	{
	    return FALSE;
	}
	BlockHandlerRegistered = TRUE;
    }
    pRequest = (DisplayRequestPtr) xalloc (sizeof (DisplayRequestRec));
    if (!pRequest)
	return FALSE;
    pRequest->pClient = client;
    pRequest->activateTime = activateTime;
    pRequest->id = FakeClientID (client->index);
    if (!AddResource (pRequest->id, DisplayRequestResType, (pointer) pRequest))
    {
	xfree (pRequest);
	return FALSE;
    }
    /* Insert into time-ordered queue, with earliest activation time coming first. */
    pPrev = 0;
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->activateTime, activateTime) == LATER)
	    break;
	pPrev = pReq;
    }
    if (pPrev)
	pPrev->next = pRequest;
    else
	pPendingRequests = pRequest;
    pRequest->next = pReq;
    if (client->swapped)
    {
    	register int    n;
    	REQUEST (xMbufDisplayImageBuffersReq);
    	
    	SwapRestL(stuff);
    	swaps (&stuff->length, n);
    	swaps (&stuff->minDelay, n);
    	swaps (&stuff->maxDelay, n);
    }
    client->sequence--;
    ResetCurrentRequest (client);
    IgnoreClient (client);
    return TRUE;
}

/*ARGSUSED*/
static void
MultibufferBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    struct timeval  **wt;		/* wait time */
    long	    *LastSelectMask;
{
    DisplayRequestPtr	    pReq;
    unsigned long	    newdelay, olddelay;
    static struct timeval   delay_val;

    if (!pPendingRequests)
	return;

    /* resume servicing clients that have display requests whose time
     * has come
     */

    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->activateTime, currentTime) == LATER)
	    break;
	AttendClient (pReq->pClient);
	FreeResource (pReq->id, 0);
    }

    /* if there are no display requests scheduled to be performed in
     * the future (later than currentTime), return
     */

    pReq = pPendingRequests;
    if (!pReq)
	return;

    /* figure out when we want select to wake up to service the next
     * display request in the queue
     */

    newdelay = pReq->activateTime.milliseconds - currentTime.milliseconds;
    if (*wt == NULL)
    { /* select wasn't going to have a timeout; now it will */
	delay_val.tv_sec = newdelay / 1000;
	delay_val.tv_usec = 1000 * (newdelay % 1000);
	*wt = &delay_val;
    }
    else
    { /* modify existing select timeout */
	olddelay = (*wt)->tv_sec * 1000 + (*wt)->tv_usec / 1000;
	if (newdelay < olddelay)
	{
	    (*wt)->tv_sec = newdelay / 1000;
	    (*wt)->tv_usec = 1000 * (newdelay % 1000);
	}
    }
}

/*ARGSUSED*/
static void
MultibufferWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    long	    *LastSelectMask;
{
    DisplayRequestPtr	pReq;

    /* if no display requests are waiting, don't come here next time */

    if (!pPendingRequests)
    {
	RemoveBlockAndWakeupHandlers (MultibufferBlockHandler,
				      MultibufferWakeupHandler,
				      (pointer) 0);
	BlockHandlerRegistered = FALSE;
	return;
    }

    /* resume servicing clients that have display requests whose time
     * has come
     */

    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->activateTime, currentTime) == LATER)
	    break;
	AttendClient (pReq->pClient);
	FreeResource (pReq->id, 0);
    }
}

/*
 * Deliver events to a buffer
 */

static int
DeliverEventsToMultibuffer (pMBBuffer, pEvents, count, filter)
    mbufBufferPtr	pMBBuffer;
    xEvent	*pEvents;
    int		count;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;

    if (!((pMBBuffer->otherEventMask|pMBBuffer->eventMask) & filter))
	return 0;
    if (attempt = TryClientEvents(
	bClient(pMBBuffer), pEvents, count, pMBBuffer->eventMask, filter, (GrabPtr) 0))
    {
	if (attempt > 0)
	    deliveries++;
	else
	    nondeliveries--;
    }
    for (other = pMBBuffer->otherClients; other; other=other->next)
    {
	if (attempt = TryClientEvents(
	      rClient(other), pEvents, count, other->mask, filter, (GrabPtr) 0))
	{
	    if (attempt > 0)
		deliveries++;
	    else
		nondeliveries--;
	}
    }
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/*
 * Send Expose events to interested clients
 */

void
MultibufferExpose (pMBBuffer, pRegion)
    mbufBufferPtr	pMBBuffer;
    RegionPtr	pRegion;
{
    if (pRegion && !REGION_NIL(pRegion))
    {
	xEvent *pEvent;
	DrawablePtr   pDrawable;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;

	pDrawable = pMBBuffer->pDrawable;
	(* pDrawable->pScreen->TranslateRegion)(pRegion,
		    -pDrawable->x, -pDrawable->y);
	/* XXX MultibufferExpose "knows" the region representation */
	numRects = REGION_NUM_RECTS(pRegion);
	pBox = REGION_RECTS(pRegion);

	pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent));
	if (pEvent) {
	    pe = pEvent;

	    for (i=1; i<=numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pDrawable->id;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (numRects - i);
	    }
	    (void) DeliverEventsToMultibuffer (pMBBuffer, pEvent, numRects, ExposureMask);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

void
MultibufferUpdate (pMBBuffer, time)
    mbufBufferPtr	pMBBuffer;
    CARD32	time;
{
    xMbufUpdateNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferUpdateNotify;
    event.buffer = pMBBuffer->pDrawable->id;
    event.timeStamp = time;
    (void) DeliverEventsToMultibuffer (pMBBuffer, (xEvent *)&event,
				       1, MultibufferUpdateNotifyMask);
}

void
MultibufferClobber (pMBBuffer)
    mbufBufferPtr	pMBBuffer;
{
    xMbufClobberNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferClobberNotify;
    event.buffer = pMBBuffer->pDrawable->id;
    event.state = pMBBuffer->clobber;
    (void) DeliverEventsToMultibuffer (pMBBuffer, (xEvent *)&event,
				       1, MultibufferClobberNotifyMask);
}

/*
 * make the resource id for buffer i refer to the window
 * drawable instead of the pixmap;
 */

static void
AliasMultibuffer (pMBWindow, i)
    mbufWindowPtr	pMBWindow;
    int		i;
{
    mbufBufferPtr	pMBBuffer;

    if (i == pMBWindow->displayedMultibuffer)
	return;
    /*
     * remove the old association
     */
    if (pMBWindow->displayedMultibuffer >= 0)
    {
	pMBBuffer = &pMBWindow->buffers[pMBWindow->displayedMultibuffer];
	ChangeResourceValue (pMBBuffer->pDrawable->id,
			     MultibufferDrawableResType,
 			     (pointer) pMBBuffer->pDrawable);
    }
    /*
     * make the new association
     */
    pMBBuffer = &pMBWindow->buffers[i];
    ChangeResourceValue (pMBBuffer->pDrawable->id,
			 MultibufferDrawableResType,
			 (pointer) pMBWindow->pWindow);
    pMBWindow->displayedMultibuffer = i;
}

static Bool
MultibuffersDestroyWindow(pWin)
    WindowPtr pWin;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufScreenPtr pMBScreen = MB_SCREEN_PRIV(pScreen);
    Bool ret;

    UNWRAP_SCREEN_FUNC(pScreen, pMBScreen, Bool, DestroyWindow);
    ret = (* pScreen->DestroyWindow)(pWin);

    if (MB_WINDOW_PRIV(pWin))
	DestroyImageBuffers(pWin);

    REWRAP_SCREEN_FUNC(pScreen, pMBScreen, Bool, DestroyWindow);
    return (ret);
}

/*
 * free everything associated with multibuffering for this
 * window
 */

void
DestroyImageBuffers (pWin)
    WindowPtr	pWin;
{
    ScreenPtr pScreen;
    mbufScreenPtr pMBScreen;
    mbufWindowPtr pMBWindow;
    int	i;

    if (!(pMBWindow = MB_WINDOW_PRIV(pWin)))
	return;

    pScreen   = pWin->drawable.pScreen;
    pMBScreen = MB_SCREEN_PRIV(pScreen);

    (* pMBScreen->DestroyImageBuffers)(pWin);

    for (i = 0; i < pMBWindow->numMultibuffer; i++)
    {
	DrawablePtr pDrawable = pMBWindow->buffers[i].pDrawable;
	FreeResource (pDrawable->id, MultibufferDrawableResType);
	(* pMBScreen->DeleteBufferDrawable)(pDrawable);
    }
    xfree (pMBWindow->buffers);
    xfree (pMBWindow);

    pWin->devPrivates[MultibufferWindowIndex].ptr = NULL;

#ifdef DEBUG
    if (pMBScreen->mbufWindowCount == 0)
	FatalError("DestroyImageBuffers: count underflowed");
#endif
    pMBScreen->mbufWindowCount--;
}

/* Resource delete func for MultibufferDrawableResType */
/*ARGSUSED*/
static void
MultibufferDrawableDelete (pDrawable, id)
    DrawablePtr	pDrawable;
    XID		id;
{
    /* Destroyed by DestroyImageBuffers when window is deleted */
    return;
}

/* Resource delete func for MultibufferResType */
/*ARGSUSED*/
static void
MultibufferDelete(pMBBuffer, id)
    mbufBufferPtr pMBBuffer;
    XID         id;
{
    return;
}

/* Resource delete func for DisplayRequestResType */
/*ARGSUSED*/
static void
DisplayRequestDelete (pRequest, id)
    DisplayRequestPtr	pRequest;
    XID			id;
{
    DisposeDisplayRequest (pRequest);
}

/* Resource delete func for OtherClientResType */
static void
OtherClientDelete (pMBBuffer, id)
    mbufBufferPtr	pMBBuffer;
    XID		id;
{
    register OtherClientsPtr	other, prev;

    prev = 0;
    for (other = pMBBuffer->otherClients; other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
		pMBBuffer->otherClients = other->next;
	    xfree (other);
	    RecalculateMultibufferOtherEvents (pMBBuffer);
	    break;
	}
	prev = other;
    }
}

static int
EventSelectForMultibuffer (pMBBuffer, client, mask)
    mbufBufferPtr	pMBBuffer;
    ClientPtr	client;
    Mask	mask;
{
    OtherClientsPtr	other;

    if (mask & ~ValidEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (bClient (pMBBuffer) == client)
    {
	pMBBuffer->eventMask = mask;
    }
    else
    {
	for (other = pMBBuffer->otherClients; other; other = other->next)
	{
	    if (SameClient (other, client))
	    {
		if (mask == 0)
		{
		    FreeResource (other->resource, RT_NONE);
		    break;
		}
		other->mask = mask;
		break;
	    }
	}
	if (!other)
	{
	    other = (OtherClients *) xalloc (sizeof (OtherClients));
	    if (!other)
		return BadAlloc;
	    other->mask = mask;
	    other->resource = FakeClientID (client->index);
	    if (!AddResource (other->resource, OtherClientResType, (pointer) pMBBuffer))
	    {
		xfree (other);
		return BadAlloc;
	    }
	    other->next = pMBBuffer->otherClients;
	    pMBBuffer->otherClients = other;
	}
	RecalculateMultibufferOtherEvents (pMBBuffer);
    }
    return (client->noClientException);
}

static void
RecalculateMultibufferOtherEvents (pMBBuffer)
    mbufBufferPtr	pMBBuffer;
{
    Mask	    otherEventMask;
    OtherClients    *other;

    otherEventMask = 0L;
    for (other = pMBBuffer->otherClients; other; other = other->next)
	otherEventMask |= other->mask;
    pMBBuffer->otherEventMask = otherEventMask;
}

/* add milliseconds to a timestamp */
static void
BumpTimeStamp (ts, inc)
TimeStamp   *ts;
CARD32	    inc;
{
    CARD32  newms;

    newms = ts->milliseconds + inc;
    if (newms < ts->milliseconds)
	ts->months++;
    ts->milliseconds = newms;
}
