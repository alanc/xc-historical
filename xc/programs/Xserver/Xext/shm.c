/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

                    All Rights Reserved

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

********************************************************/

/* $XConsortium: shm.c,v 1.0 89/08/18 17:54:44 rws Exp $ */

#ifdef MITSHM

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "extnsionst.h"
#include "servermd.h"
#include "shmstr.h"

typedef struct _ShmDesc {
    struct _ShmDesc *next;
    int shmid;
    int refcnt;
    char *addr;
    Bool writable;
    unsigned long size;
} ShmDescRec, *ShmDescPtr;

char *shmat();
typedef PixmapPtr (*ShmCreatePixType)();

Bool ShmSharedPixmaps = xFalse;
ShmCreatePixType ShmCreatePixProcs[MAXSCREENS];

static unsigned char ShmReqCode;
static int ShmCompletionCode;
static int BadShmSegCode;
static RESTYPE ShmSegType, ShmPixType;
static ShmDescPtr Shmsegs;

#define VALIDATE_SHMSEG(shmseg,shmdesc,client) \
{ \
    shmdesc = (ShmDescPtr)LookupIDByType(shmseg, ShmSegType); \
    if (!shmdesc) \
    { \
	client->errorValue = shmseg; \
	return BadShmSegCode; \
    } \
}

#define VALIDATE_SHMPTR(shmseg,offset,len,needwrite,shmdesc,client) \
{ \
    VALIDATE_SHMSEG(shmseg, shmdesc, client); \
    if ((offset & 3) || (offset >= shmdesc->size)) \
    { \
	client->errorValue = offset; \
	return BadValue; \
    } \
    if ((offset + len) > shmdesc->size) \
    { \
	client->errorValue = len; \
	return BadValue; \
    } \
    if (needwrite && !shmdesc->writable) \
	return BadAccess; \
}

void
ShmExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();
    static int ProcShmDispatch(), SProcShmDispatch();
    static int ShmDetachSegment();
    static void ShmResetProc(), SShmCompletionEvent();

    ShmSegType = CreateNewResourceType(ShmDetachSegment);
    ShmPixType = CreateNewResourceType(ShmDetachSegment);
    if (ShmSegType && ShmPixType &&
	(extEntry = AddExtension(SHMNAME, ShmNumberEvents, ShmNumberErrors,
				 ProcShmDispatch, SProcShmDispatch,
				 ShmResetProc, StandardMinorOpcode)))
    {
	ShmReqCode = (unsigned char)extEntry->base;
	ShmCompletionCode = extEntry->eventBase;
	BadShmSegCode = extEntry->errorBase;
	EventSwapVector[ShmCompletionCode] = SShmCompletionEvent;
    }
}

/*ARGSUSED*/
static void
ShmResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

void
ShmRegisterCreatePixmapProc(pScreen, func)
    ScreenPtr pScreen;
    ShmCreatePixType func;
{
    ShmCreatePixProcs[pScreen->myNum] = func;
    ShmSharedPixmaps = xTrue;
}

static int
ProcShmQueryVersion(client)
    register ClientPtr client;
{
    REQUEST(xShmQueryVersionReq);
    xShmQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xShmQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.sharedPixmaps = ShmSharedPixmaps;
    rep.majorVersion = SHM_MAJOR_VERSION;
    rep.minorVersion = SHM_MINOR_VERSION;
    rep.uid = geteuid();
    rep.gid = getegid();
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
	swaps(&rep.uid, n);
	swaps(&rep.gid, n);
    }
    WriteToClient(client, sizeof(xShmQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcShmAttach(client)
    register ClientPtr client;
{
    struct shmid_ds buf;
    ShmDescPtr shmdesc;
    REQUEST(xShmAttachReq);

    REQUEST_SIZE_MATCH(xShmAttachReq);
    LEGAL_NEW_RESOURCE(stuff->shmseg, client);
    if ((stuff->readOnly != xTrue) && (stuff->readOnly != xFalse))
    {
	client->errorValue = stuff->readOnly;
        return(BadValue);
    }
    for (shmdesc = Shmsegs;
	 shmdesc && (shmdesc->shmid != stuff->shmid);
	 shmdesc = shmdesc->next)
	;
    if (shmdesc)
    {
	if (!stuff->readOnly && !shmdesc->writable)
	    return BadAccess;
	shmdesc->refcnt++;
    }
    else
    {
	shmdesc = (ShmDescPtr) xalloc(sizeof(ShmDescRec));
	if (!shmdesc)
	    return BadAlloc;
	shmdesc->addr = shmat(stuff->shmid, 0,
			      stuff->readOnly ? SHM_RDONLY : 0);
	if ((shmdesc->addr == ((char *)-1)) ||
	    shmctl(stuff->shmid, IPC_STAT, &buf))
	{
	    xfree(shmdesc);
	    return BadAccess;
	}
	shmdesc->shmid = stuff->shmid;
	shmdesc->refcnt = 1;
	shmdesc->writable = !stuff->readOnly;
	shmdesc->size = buf.shm_segsz;
	shmdesc->next = Shmsegs;
	Shmsegs = shmdesc;
    }
    if (!AddResource(stuff->shmseg, ShmSegType, (pointer)shmdesc))
	return BadAlloc;
    return(client->noClientException);
}

/*ARGSUSED*/
static int
ShmDetachSegment(shmdesc, shmseg)
    ShmDescPtr shmdesc;
    ShmSeg shmseg;
{
    ShmDescPtr *prev;

    if (--shmdesc->refcnt)
	return TRUE;
    shmdt(shmdesc->addr);
    for (prev = &Shmsegs; *prev != shmdesc; prev = &(*prev)->next)
	;
    *prev = shmdesc->next;
    xfree(shmdesc);
}

static int
ProcShmDetach(client)
    register ClientPtr client;
{
    ShmDescPtr shmdesc;
    REQUEST(xShmDetachReq);

    REQUEST_SIZE_MATCH(xShmDetachReq);
    VALIDATE_SHMSEG(stuff->shmseg, shmdesc, client);
    FreeResource(stuff->shmseg, RT_NONE);
    return(client->noClientException);
}

static int
ProcShmPutImage(client)
    register ClientPtr client;
{
    register GCPtr pGC;
    register DrawablePtr pDraw;
    long length;
    ShmDescPtr shmdesc;
    REQUEST(xShmPutImageReq);

    REQUEST_SIZE_MATCH(xShmPutImageReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    VALIDATE_SHMPTR(stuff->shmseg, stuff->offset, stuff->size,
		    FALSE, shmdesc, client);
    if ((stuff->sendEvent != xTrue) && (stuff->sendEvent != xFalse))
	return BadValue;
    if (stuff->format == XYBitmap)
    {
        if (stuff->depth != 1)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, 1);
    }
    else if (stuff->format == XYPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, 1);
	length *= stuff->depth;
    }
    else if (stuff->format == ZPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, stuff->depth);
    }
    else
    {
	client->errorValue = stuff->format;
        return BadValue;
    }
    if ((length * stuff->totalHeight) != stuff->size)
    {
	client->errorValue = stuff->size;
	return BadValue;
    }
    if (stuff->srcX > stuff->totalWidth)
    {
	client->errorValue = stuff->srcX;
	return BadValue;
    }
    if (stuff->srcY > stuff->totalHeight)
    {
	client->errorValue = stuff->srcY;
	return BadValue;
    }
    if ((stuff->srcX + stuff->srcWidth) > stuff->totalWidth)
    {
	client->errorValue = stuff->srcWidth;
	return BadValue;
    }
    if ((stuff->srcY + stuff->srcHeight) > stuff->totalHeight)
    {
	client->errorValue = stuff->srcHeight;
	return BadValue;
    }
    if ((((stuff->format == ZPixmap) && (stuff->srcX == 0)) ||
	 ((stuff->format != ZPixmap) &&
	  (stuff->srcX < screenInfo.bitmapScanlinePad) &&
	  ((stuff->format == XYBitmap) ||
	   ((stuff->srcY == 0) &&
	    (stuff->srcHeight == stuff->totalHeight))))) &&
	((stuff->srcX + stuff->srcWidth) == stuff->totalWidth))
    {
	(*pGC->ops->PutImage) (pDraw, pGC, stuff->depth,
			       stuff->dstX, stuff->dstY,
			       stuff->totalWidth, stuff->srcHeight, 
			       stuff->srcX, stuff->format, 
			       shmdesc->addr + stuff->offset +
			       (stuff->srcY * length));
    }
    else if ((stuff->format == ZPixmap) || (stuff->depth == 1))
    {
	/* XXX massive assumptions for now */
	PixmapRec FakePixmap;
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDraw->pScreen;
    	FakePixmap.drawable.depth = stuff->depth;
    	FakePixmap.drawable.bitsPerPixel = stuff->depth;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = stuff->totalWidth;
    	FakePixmap.drawable.height = stuff->totalHeight;
    	FakePixmap.devKind = PixmapBytePad(stuff->totalWidth, stuff->depth);
    	FakePixmap.refcnt = 1;
    	FakePixmap.devPrivate.ptr = (pointer)(shmdesc->addr + stuff->offset);
	if (stuff->format == XYBitmap)
	    (void)(*pGC->ops->CopyPlane)(&FakePixmap, pDraw, pGC,
					 stuff->srcX, stuff->srcY,
					 stuff->srcWidth, stuff->srcHeight, 
					 stuff->dstX, stuff->dstY, 1L);
	else
	    (void)(*pGC->ops->CopyArea)(&FakePixmap, pDraw, pGC,
					stuff->srcX, stuff->srcY,
					stuff->srcWidth, stuff->srcHeight, 
					stuff->dstX, stuff->dstY);
    }
    else
    {
	/* XXX */
    }
    if (stuff->sendEvent)
    {
	xShmCompletionEvent ev;

	ev.type = ShmCompletionCode;
	ev.drawable = stuff->drawable;
	ev.sequenceNumber = client->sequence;
	ev.minorEvent = X_ShmPutImage;
	ev.majorEvent = ShmReqCode;
	ev.shmseg = stuff->shmseg;
	ev.offset = stuff->offset;
	WriteEventsToClient(client, 1, (xEvent *) &ev);
    }
     return (client->noClientException);
}

static int
ProcShmGetImage(client)
    register ClientPtr client;
{
    REQUEST(xShmGetImageReq);

    REQUEST_SIZE_MATCH(xShmGetImageReq);
}

static int
ProcShmCreatePixmap(client)
    register ClientPtr client;
{
    PixmapPtr pMap;
    register DrawablePtr pDraw;
    DepthPtr pDepth;
    register int i;
    ShmDescPtr shmdesc;
    REQUEST(xShmCreatePixmapReq);

    REQUEST_SIZE_MATCH(xShmCreatePixmapReq);
    client->errorValue = stuff->pid;
    if (!ShmSharedPixmaps)
	return BadImplementation;
    LEGAL_NEW_RESOURCE(stuff->pid, client);
    if (!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client)))
    {        /* can be inputonly */
        if (!(pDraw = (DrawablePtr)LookupWindow(stuff->drawable, client))) 
            return (BadDrawable);
    }

    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    if (stuff->depth != 1)
    {
        pDepth = pDraw->pScreen->allowedDepths;
        for (i=0; i<pDraw->pScreen->numDepths; i++, pDepth++)
	   if (pDepth->depth == stuff->depth)
               goto CreatePmap;
	client->errorValue = stuff->depth;
        return BadValue;
    }
CreatePmap:
    VALIDATE_SHMPTR(stuff->shmseg, stuff->offset,
		    PixmapBytePad(stuff->width, stuff->depth) * stuff->height,
		    TRUE, shmdesc, client);
    pMap = (*ShmCreatePixProcs[pDraw->pScreen->myNum])(
			    pDraw->pScreen, stuff->width,
			    stuff->height, stuff->depth,
			    shmdesc->addr + stuff->offset);
    if (pMap)
    {
	pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pMap->drawable.id = stuff->pid;
	if (AddResource(stuff->pid, RT_PIXMAP, (pointer)pMap))
	{
	    shmdesc->refcnt++;
	    if (AddResource(stuff->pid, ShmPixType, (pointer)shmdesc))
		return(client->noClientException);
	    FreeResource(stuff->pid, RT_NONE);
	}
    }
    return (BadAlloc);
}

static int
ProcShmDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_ShmQueryVersion:
	return ProcShmQueryVersion(client);
    case X_ShmAttach:
	return ProcShmAttach(client);
    case X_ShmDetach:
	return ProcShmDetach(client);
    case X_ShmPutImage:
	return ProcShmPutImage(client);
    case X_ShmGetImage:
	return ProcShmGetImage(client);
    case X_ShmCreatePixmap:
	return ProcShmCreatePixmap(client);
    default:
	return BadRequest;
    }
}

static void
SShmCompletionEvent(from, to)
    xShmCompletionEvent *from, *to;
{
    to->type = from->type;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->drawable, to->drawable);
    cpswaps(from->minorEvent, to->minorEvent);
    to->majorEvent = from->majorEvent;
    cpswapl(from->shmseg, to->shmseg);
    cpswapl(from->offset, to->offset);
}

static int
SProcShmQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xShmQueryVersionReq);

    swaps(&stuff->length, n);
    return ProcShmQueryVersion(client);
}

static int
SProcShmAttach(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmAttachReq);
    swaps(&stuff->length, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->shmid, n);
    return ProcShmAttach(client);
}

static int
SProcShmDetach(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmDetachReq);
    swaps(&stuff->length, n);
    swapl(&stuff->shmseg, n);
    return ProcShmDetach(client);
}

static int
SProcShmPutImage(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmPutImageReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->totalWidth, n);
    swaps(&stuff->totalHeight, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->srcWidth, n);
    swaps(&stuff->srcHeight, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    swapl(&stuff->size, n);
    return ProcShmPutImage(client);
}

static int
SProcShmGetImage(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmGetImageReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->planeMask, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    swapl(&stuff->size, n);
    return ProcShmGetImage(client);
}

static int
SProcShmCreatePixmap(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmCreatePixmapReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    swapl(&stuff->size, n);
    return ProcShmCreatePixmap(client);
}

static int
SProcShmDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_ShmQueryVersion:
	return SProcShmQueryVersion(client);
    case X_ShmAttach:
	return SProcShmAttach(client);
    case X_ShmDetach:
	return SProcShmDetach(client);
    case X_ShmPutImage:
	return SProcShmPutImage(client);
    case X_ShmGetImage:
	return SProcShmGetImage(client);
    case X_ShmCreatePixmap:
	return SProcShmCreatePixmap(client);
    default:
	return BadRequest;
    }
}

#endif
