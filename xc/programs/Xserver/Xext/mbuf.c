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

/* $XConsortium: buffer.c,v 1.2 89/09/21 11:08:24 keith Exp $ */
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
#include "inputstr.h"
#include <sys/time.h>

/*
 * per-buffer data
 */
 
typedef struct _Buffers	*BuffersPtr;

#define SameClient(obj,client) \
	(CLIENT_BITS((obj)->resource) == (client)->clientAsMask)
#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])
#define bClient(b)   (clients[CLIENT_ID(b->pPixmap->drawable.id)])

#define ValidEventMasks (ExposureMask|ClobberNotifyMask|UpdateNotifyMask)

typedef struct _Buffer {
    BuffersPtr	pBuffers;	/* associated window data */
    Mask	eventMask;	/* ClobberNotifyMask|ExposureMask|UpdateNotifyMask */
    Mask	otherEventMask;	/* mask of all other clients event masks */
    OtherClients    *otherClients;
    int		number;		/* index into array */
    int		side;		/* alwys Mono */
    int		clobber;	/* Unclobbered, PartiallyClobbered, FullClobbered */
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

    TimeStamp	lastUpdate;	/* time of last update */

    unsigned short	width, height;	/* last known window size */
    short		x, y;		/* for static gravity */

    BufferPtr	buffers;
} BuffersRec;

/*
 * per-screen data
 */
typedef struct _BufferScreen {
    Bool    (*PositionWindow)();
} BufferScreenRec, *BufferScreenPtr;

/*
 * per display-image-buffers request data.
 */

typedef struct _DisplayRequest {
    struct _DisplayRequest	*next;
    TimeStamp			activateTime;
    ClientPtr			pClient;
    XID				id;
} DisplayRequestRec, *DisplayRequestPtr;

static unsigned char	BufferReqCode;
static int		BufferEventBase;
static int		BufferErrorBase;
static int		BufferScreenIndex = -1;
static int		BufferWindowIndex = -1;

static int		BlockHandlerRegistered;
static void		BufferBlockHandler(), BufferWakeupHandler();

static void		PerformDisplayRequest ();
static void		DisposeDisplayRequest ();
static Bool		QueueDisplayRequest ();

static void		BumpTimeStamp ();

static void		BufferExpose ();
static void		BufferUpdate ();
static void		AliasBuffer ();
static void		DisposeBuffers ();
static void		RecalculateBufferOtherEvents ();
static int		EventSelectForBuffer();

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
static RESTYPE	BufferDrawableResType;
static void	BufferDrawableDelete ();
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
static RESTYPE	BufferResType;
static void	BufferDelete ();
/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
static RESTYPE	BuffersResType;
static void	BuffersDelete ();
/*
 * Per display-buffers request is attached to a resource so that
 * it will disappear if the client dies before the request should
 * be processed
 */
static RESTYPE	DisplayRequestResType;
static void	DisplayRequestDelete ();
/*
 * Clients other than the buffer creator attach event masks in
 * OtherClient structures; each has a resource of this type.
 */
static RESTYPE	OtherClientResType;
static void	OtherClientDelete ();

/****************
 * BufferExtensionInit
 *
 * Called from InitExtensions in main()
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
     * create the resource types
     */
    BufferDrawableResType =
	CreateNewResourceType(BufferDrawableDelete)|RC_CACHED|RC_DRAWABLE;
    BufferResType = CreateNewResourceType(BufferDelete);
    BuffersResType = CreateNewResourceType(BuffersDelete);
    DisplayRequestResType = CreateNewResourceType(DisplayRequestDelete);
    OtherClientResType = CreateNewResourceType(OtherClientDelete);
    if (BufferDrawableResType && BufferResType &&
	BuffersResType && DisplayRequestResType &&
	OtherClientResType &&
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

static int
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
	pBuffer->eventMask = 0L;
	pBuffer->otherEventMask = 0L;
	pBuffer->otherClients = (OtherClientsPtr) NULL;
	pBuffer->number = i;
	pBuffer->side = BufferSideMono;
	pBuffer->clobber = Unclobbered;
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
    pBuffers->lastUpdate.months = 0;
    pBuffers->lastUpdate.milliseconds = 0;
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
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    CARD32	    minDelay, maxDelay;
    TimeStamp	    activateTime, bufferTime;
    
    REQUEST_AT_LEAST_SIZE (xDisplayImageBuffersReq);
    nbuf = stuff->length - (sizeof (xDisplayImageBuffersReq) >> 2);
    if (!nbuf)
	return Success;
    minDelay = stuff->minDelay;
    maxDelay = stuff->maxDelay;
    ids = (XID *) &stuff[1];
    ppBuffers = (BuffersPtr *) xalloc (nbuf * sizeof (BuffersPtr));
    pBuffer = (BufferPtr *) xalloc (nbuf * sizeof (BufferPtr));
    if (!ppBuffers || !pBuffer)
    {
	xfree (ppBuffers);
	xfree (pBuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    activateTime.months = 0;
    activateTime.milliseconds = 0;
    for (i = 0; i < nbuf; i++)
    {
	pBuffer[i] = (BufferPtr) LookupIDByType (ids[i], BufferResType);
	if (!pBuffer[i])
	{
	    xfree (ppBuffers);
	    xfree (pBuffer);
	    client->errorValue = ids[i];
	    return BufferErrorBase + BufferError;
	}
	ppBuffers[i] = pBuffer[i]->pBuffers;
	for (j = 0; j < i; j++)
	{
	    if (ppBuffers[i] == ppBuffers[j])
	    {
	    	xfree (ppBuffers);
	    	xfree (pBuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}
	bufferTime = ppBuffers[i]->lastUpdate;
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
	PerformDisplayRequest (ppBuffers, pBuffer, nbuf);
    xfree (ppBuffers);
    xfree (pBuffer);
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
ProcSetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST (xSetMultiBufferAttributesReq);
    WindowPtr	pWin;
    BuffersPtr	pBuffers;
    int		len;
    Mask	vmask;
    Mask	index;
    CARD32	updateHint;
    XID		*vlist;

    REQUEST_AT_LEAST_SIZE (xSetMultiBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pBuffers = (BuffersPtr)LookupIDByType (pWin->drawable.id, BuffersResType);
    if (!pBuffers)
	return BadMatch;
    len = stuff->length - (sizeof (xSetMultiBufferAttributesReq) >> 2);
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
	case SMBUpdateHint:
	    updateHint = (CARD32) *vlist;
	    switch (updateHint)
	    {
	    case UpdateHintFrequent:
	    case UpdateHintIntermittent:
	    case UpdateHintStatic:
		pBuffers->updateHint = updateHint;
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
    return Success;
}

static int
ProcGetMultiBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xGetMultiBufferAttributesReq);
    WindowPtr	pWin;
    BuffersPtr	pBuffers;
    XID		*ids;
    xGetMultiBufferAttributesReply  rep;
    int		i, n;

    REQUEST_SIZE_MATCH (xGetMultiBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pBuffers = (BuffersPtr)LookupIDByType (pWin->drawable.id, BuffersResType);
    if (!pBuffers)
	return BadAccess;
    ids = (XID *) ALLOCATE_LOCAL (pBuffers->numBuffer * sizeof (XID));
    if (!ids)
	return BadAlloc;
    for (i = 0; i < pBuffers->numBuffer; i++)
	ids[i] = pBuffers->buffers[i].pPixmap->drawable.id;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = pBuffers->numBuffer;
    rep.displayedBuffer = pBuffers->displayedBuffer;
    rep.updateAction = pBuffers->updateAction;
    rep.updateHint = pBuffers->updateHint;
    rep.windowMode = pBuffers->windowMode;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.displayedBuffer, n);
	SwapLongs (ids, pBuffers->numBuffer);
    }
    WriteToClient (client, sizeof (xGetMultiBufferAttributesReply), &rep);
    WriteToClient (client, (int) (pBuffers->numBuffer * sizeof (XID)), ids);
    DEALLOCATE_LOCAL((pointer) ids);
    return client->noClientException;
}

static int
ProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xSetBufferAttributesReq);
    BufferPtr	pBuffer;
    int		len;
    Mask	vmask, index;
    XID		*vlist;
    Mask	eventMask;
    int		result;

    REQUEST_AT_LEAST_SIZE (xSetBufferAttributesReq);
    pBuffer = (BufferPtr) LookupIDByType (stuff->buffer, BufferResType);
    if (!pBuffer)
	return BufferErrorBase + BufferError;
    len = stuff->length - (sizeof (xSetBufferAttributesReq) >> 2);
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
	case SBEventMask:
	    eventMask = (Mask) *vlist;
	    vlist++;
	    result = EventSelectForBuffer (pBuffer, client, eventMask);
	    if (result != Success)
		return result;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
	    return BadValue;
	}
    }
    return Success;
}

ProcGetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xGetBufferAttributesReq);
    BufferPtr	pBuffer;
    xGetBufferAttributesReply	rep;
    OtherClientsPtr		other;
    int				n;

    REQUEST_SIZE_MATCH (xGetBufferAttributesReq);
    pBuffer = (BufferPtr) LookupIDByType (stuff->buffer, BufferResType);
    if (!pBuffer)
	return BufferErrorBase + BufferError;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.window = pBuffer->pBuffers->pWindow->drawable.id;
    if (bClient (pBuffer) == client)
	rep.eventMask = pBuffer->eventMask;
    else
    {
	rep.eventMask = (Mask) 0L;
	for (other = pBuffer->otherClients; other; other = other->next)
	    if (SameClient (other, client))
	    {
		rep.eventMask = other->mask;
		break;
	    }
    }
    rep.index = pBuffer->number;
    rep.side = pBuffer->side;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.window, n);
	swapl(&rep.eventMask, n);
	swaps(&rep.index, n);
    }
    WriteToClient(client, sizeof (xGetBufferAttributesReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcGetBufferInfo (client)
    register ClientPtr	client;
{
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
    case X_SetMultiBufferAttributes:
	return ProcSetMultiBufferAttributes (client);
    case X_GetMultiBufferAttributes:
	return ProcGetMultiBufferAttributes (client);
    case X_SetBufferAttributes:
	return ProcSetBufferAttributes (client);
    case X_GetBufferAttributes:
	return ProcGetBufferAttributes (client);
    case X_GetBufferInfo:
	return ProcGetBufferInfo (client);
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
SProcSetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xSetMultiBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSetMultiBufferAttributesReq);
    swapl (&stuff->window, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetMultiBufferAttributes (client);
}

static int
SProcGetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xGetMultiBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xGetMultiBufferAttributesReq);
    swapl (&stuff->window, n);
    return ProcGetMultiBufferAttributes (client);
}

static int
SProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xSetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSetBufferAttributesReq);
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
    REQUEST (xGetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xGetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    return ProcGetBufferAttributes (client);
}

static int
SProcGetBufferInfo (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xGetBufferInfoReq);

    swaps (&stuff->length, n);
    return ProcGetBufferInfo (client);
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
    case X_SetMultiBufferAttributes:
	return SProcSetMultiBufferAttributes (client);
    case X_GetMultiBufferAttributes:
	return SProcGetMultiBufferAttributes (client);
    case X_SetBufferAttributes:
	return SProcSetBufferAttributes (client);
    case X_GetBufferAttributes:
	return SProcGetBufferAttributes (client);
    case X_GetBufferInfo:
	return SProcGetBufferInfo (client);
    default:
	return BadRequest;
    }
}

static void
SUpdateNotifyEvent (from, to)
    xUpdateNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    cpswapl (from->timeStamp, to->timeStamp);
}

static void
SClobberNotifyEvent (from, to)
    xClobberNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    to->state = from->state;
}

static void
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

static void
PerformDisplayRequest (ppBuffers, pBuffer, nbuf)
    BufferPtr	    *pBuffer;
    BuffersPtr	    *ppBuffers;
    int		    nbuf;
{
    GCPtr	    pGC;
    PixmapPtr	    pPrevPixmap, pNewPixmap;
    xRectangle	    clearRect;
    WindowPtr	    pWin;
    RegionPtr	    pExposed;
    int		    i;
    BufferPtr	    pPrevBuffer;
    XID		    bool;

    UpdateCurrentTime ();
    for (i = 0; i < nbuf; i++)
    {
	pWin = ppBuffers[i]->pWindow;
	pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
	pPrevBuffer = &ppBuffers[i]->buffers[ppBuffers[i]->displayedBuffer];
	pPrevPixmap = pPrevBuffer->pPixmap;
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
	    if (pPrevBuffer->eventMask & ExposureMask)
	    {
	    	bool = TRUE;
	    	DoChangeGC (pGC, GCGraphicsExposures, &bool, FALSE);
	    }
	    ValidateGC (pPrevPixmap, pGC);
	    pExposed = (*pGC->ops->CopyArea)
			    ((DrawablePtr) pWin,
			     (DrawablePtr) pPrevPixmap,
			     pGC,
			     0, 0,
			     pWin->drawable.width, pWin->drawable.height,
			     0, 0);
	    if (pPrevBuffer->eventMask & ExposureMask)
	    {
	    	if (pExposed)
	    	{
		    RegionPtr	pWinSize;
		    extern RegionPtr	CreateUnclippedWinSize();

		    pWinSize = CreateUnclippedWinSize (pWin);
		    (*pWin->drawable.pScreen->Intersect) (pExposed,
							  pExposed, pWinSize);
		    (*pWin->drawable.pScreen->RegionDestroy) (pWinSize);
	    	    BufferExpose (pPrevBuffer, pExposed);
	    	    (*pWin->drawable.pScreen->RegionDestroy) (pExposed);
	    	}
	    	bool = FALSE;
	    	DoChangeGC (pGC, GCGraphicsExposures, &bool, FALSE);
	    }
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
	ppBuffers[i]->lastUpdate = currentTime;
	BufferUpdate (pBuffer[i], ppBuffers[i]->lastUpdate.milliseconds);
	AliasBuffer (ppBuffers[i], pBuffer[i] - ppBuffers[i]->buffers);
	FreeScratchGC (pGC);
    }
    return;
}

static DisplayRequestPtr    pPendingRequests;

static void
DisposeDisplayRequest (pRequest)
    DisplayRequestPtr	pRequest;
{
    DisplayRequestPtr	pReq, pPrev;

    pPrev = 0;
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
	if (pReq == pRequest)
	{
	    if (pPrev)
		pPrev->next = pReq->next;
	    else
		pPendingRequests = pReq->next;
	    xfree (pReq);
	    break;
	}
}

static Bool
QueueDisplayRequest (client, activateTime)
    ClientPtr	    client;
    TimeStamp	    activateTime;
{
    DisplayRequestPtr	pRequest, pReq, pPrev;

    if (!BlockHandlerRegistered)
    {
	if (!RegisterBlockAndWakeupHandlers (BufferBlockHandler,
					     BufferWakeupHandler,
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
    	REQUEST (xDisplayImageBuffersReq);
    	
    	SwapRestL(stuff);
    	swaps (&stuff->length, n);
    	swaps (&stuff->minDelay, n);
    	swaps (&stuff->maxDelay, n);
    }
    ResetCurrentRequest (client);
    IgnoreClient (client);
    return TRUE;
}

static void
BufferBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    struct timeval  **wt;		/* wait time */
    long	    *LastSelectMask;
{
    DisplayRequestPtr	    pReq, pNext;
    unsigned long	    newdelay, olddelay;
    static struct timeval   delay_val;

    if (!pPendingRequests)
	return;
    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->activateTime, currentTime) == LATER)
	    break;
	AttendClient (pReq->pClient);
	FreeResource (pReq->id, 0);
    }
    pReq = pPendingRequests;
    if (!pReq)
	return;
    newdelay = pReq->activateTime.milliseconds - currentTime.milliseconds;
    if (*wt == NULL)
    {
	delay_val.tv_sec = newdelay / 1000;
	delay_val.tv_usec = 1000 * (newdelay % 1000);
	*wt = &delay_val;
    }
    else
    {
	olddelay = (*wt)->tv_sec * 1000 + (*wt)->tv_usec / 1000;
	if (newdelay < olddelay)
	{
	    (*wt)->tv_sec = newdelay / 1000;
	    (*wt)->tv_usec = 1000 * (newdelay % 1000);
	}
    }
}

static void
BufferWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    long	    *LastSelectMask;
{
    DisplayRequestPtr	pReq, pNext;

    if (!pPendingRequests)
    {
	RemoveBlockAndWakeupHandlers (BufferBlockHandler,
				      BufferWakeupHandler,
				      (pointer) 0);
	BlockHandlerRegistered = 0;
	return;
    }
    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pNext)
    {
	pNext = pReq->next;
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
DeliverEventsToBuffer (pBuffer, pEvents, count, filter)
    BufferPtr	pBuffer;
    xEvent	*pEvents;
    int		count;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;

    if (!((pBuffer->otherEventMask|pBuffer->eventMask) & filter))
	return 0;
    if (attempt = TryClientEvents(
	bClient(pBuffer), pEvents, count, pBuffer->eventMask, filter, (GrabPtr) 0))
    {
	if (attempt > 0)
	    deliveries++;
	else
	    nondeliveries--;
    }
    for (other = pBuffer->otherClients; other; other=other->next)
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

static void
BufferExpose (pBuffer, pRegion)
    BufferPtr	pBuffer;
    RegionPtr	pRegion;
{
    if (pRegion && !REGION_NIL(pRegion))
    {
	xEvent *pEvent;
	PixmapPtr   pPixmap;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;

	pPixmap = pBuffer->pPixmap;
	(* pPixmap->drawable.pScreen->TranslateRegion)(pRegion,
		    -pPixmap->drawable.x, -pPixmap->drawable.y);
	/* XXX BufferExpose "knows" the region representation */
	numRects = REGION_NUM_RECTS(pRegion);
	pBox = REGION_RECTS(pRegion);

	pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent));
	if (pEvent) {
	    pe = pEvent;

	    for (i=1; i<=numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pPixmap->drawable.id;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (numRects - i);
	    }
	    (void) DeliverEventsToBuffer (pBuffer, pEvent, numRects, ExposureMask);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

static void
BufferUpdate (pBuffer, time)
    BufferPtr	pBuffer;
    CARD32	time;
{
    xUpdateNotifyEvent	event;

    event.type = BufferEventBase + UpdateNotify;
    event.buffer = pBuffer->pPixmap->drawable.id;
    event.timeStamp = time;
    (void) DeliverEventsToBuffer (pBuffer, &event, 1, UpdateNotifyMask);
}

/*
 * The sample implementation will never generate ClobberNotify
 * events
 */

static void
BufferClobber (pBuffer)
    BufferPtr	pBuffer;
{
    xClobberNotifyEvent	event;

    event.type = BufferEventBase + ClobberNotify;
    event.buffer = pBuffer->pPixmap->drawable.id;
    event.state = pBuffer->clobber;
    (void) DeliverEventsToBuffer (pBuffer, &event, 1, ClobberNotifyMask);
}

/*
 * make the resource id for buffer i refer to the window
 * drawable instead of the pixmap;
 */

static void
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

/*
 * free everything associated with multibuffering for this
 * window
 */

static void
DisposeBuffers (pWin)
    WindowPtr	pWin;
{
    FreeResourceByType (pWin->drawable.id, BuffersResType, FALSE);
}

/*
 * resize the buffers when the window is resized
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

/* Resource delete func for BufferDrawableResType */
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

/* Resource delete func for BufferResType */
static void
BufferDelete (pBuffer, id)
    BufferPtr	pBuffer;
    XID		id;
{
    return;
}

/* Resource delete func for BuffersResType */
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

/* Resource delete func for DisplayRequestResType */
static void
DisplayRequestDelete (pRequest, id)
    DisplayRequestPtr	pRequest;
    XID			id;
{
    DisposeDisplayRequest (pRequest);
}

/* Resource delete func for OtherClientResType */
static void
OtherClientDelete (pBuffer, id)
    BufferPtr	pBuffer;
    XID		id;
{
    register OtherClientsPtr	other, prev;

    prev = 0;
    for (other = pBuffer->otherClients; other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
		pBuffer->otherClients = other->next;
	    xfree (other);
	    RecalculateBufferOtherEvents (pBuffer);
	    break;
	}
	prev = other;
    }
}

static int
EventSelectForBuffer (pBuffer, client, mask)
    BufferPtr	pBuffer;
    ClientPtr	client;
    Mask	mask;
{
    OtherClientsPtr	other;

    if (mask & ~ValidEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (bClient (pBuffer) == client)
    {
	pBuffer->eventMask = mask;
    }
    else
    {
	for (other = pBuffer->otherClients; other; other = other->next)
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
	    if (!AddResource (other->resource, OtherClientResType, (pointer) pBuffer))
	    {
		xfree (other);
		return BadAlloc;
	    }
	    other->next = pBuffer->otherClients;
	    pBuffer->otherClients = other;
	}
	RecalculateBufferOtherEvents (pBuffer);
    }
}

static void
RecalculateBufferOtherEvents (pBuffer)
    BufferPtr	pBuffer;
{
    Mask	    otherEventMask;
    OtherClients    *other;

    otherEventMask = 0L;
    for (other = pBuffer->otherClients; other; other = other->next)
	otherEventMask |= other->mask;
    pBuffer->otherEventMask = otherEventMask;
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
