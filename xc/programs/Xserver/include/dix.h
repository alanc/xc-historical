/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: dix.h,v 1.84 95/03/03 01:54:22 dpw Exp $ */

#ifndef DIX_H
#define DIX_H

#include "gc.h"
#include "window.h"

#define EARLIER -1
#define SAMETIME 0
#define LATER 1

#define NullClient ((ClientPtr) 0)
#define REQUEST(type) \
	register type *stuff = (type *)client->requestBuffer


#define REQUEST_SIZE_MATCH(req)\
    if ((sizeof(req) >> 2) != client->req_len)\
         return(BadLength)

#define REQUEST_AT_LEAST_SIZE(req) \
    if ((sizeof(req) >> 2) > client->req_len )\
         return(BadLength)

#define REQUEST_FIXED_SIZE(req, n)\
    if (((sizeof(req) >> 2) > client->req_len) || \
        (((sizeof(req) + (n) + 3) >> 2) != client->req_len)) \
         return(BadLength)

#define LEGAL_NEW_RESOURCE(id,client)\
    if (!LegalNewID(id,client)) \
    {\
	client->errorValue = id;\
        return(BadIDChoice);\
    }

/* XXX if you are using this macro, you are probably not generating Match
 * errors where appropriate */
#define LOOKUP_DRAWABLE(did, client)\
    ((client->lastDrawableID == did) ? \
     client->lastDrawable : (DrawablePtr)LookupDrawable(did, client))

#define VERIFY_DRAWABLE(pDraw, did, client)\
    if (client->lastDrawableID == did)\
	pDraw = client->lastDrawable;\
    else \
    {\
	pDraw = (DrawablePtr) LookupIDByClass(did, RC_DRAWABLE);\
	if (!pDraw) \
	{\
	    client->errorValue = did; \
	    return BadDrawable;\
	}\
	if (pDraw->type == UNDRAWABLE_WINDOW)\
	    return BadMatch;\
    }

#define VERIFY_GEOMETRABLE(pDraw, did, client)\
    if (client->lastDrawableID == did)\
	pDraw = client->lastDrawable;\
    else \
    {\
	pDraw = (DrawablePtr) LookupIDByClass(did, RC_DRAWABLE);\
	if (!pDraw) \
	{\
	    client->errorValue = did; \
	    return BadDrawable;\
	}\
    }

#define VERIFY_GC(pGC, rid, client)\
    if (client->lastGCID == rid)\
        pGC = client->lastGC;\
    else\
	pGC = (GC *)LookupIDByType(rid, RT_GC);\
    if (!pGC)\
    {\
	client->errorValue = rid;\
	return (BadGC);\
    }

#define VALIDATE_DRAWABLE_AND_GC(drawID, pDraw, pGC, client)\
    if ((stuff->gc == INVALID) || (client->lastGCID != stuff->gc) ||\
	(client->lastDrawableID != drawID))\
    {\
	VERIFY_GEOMETRABLE(pDraw, drawID, client);\
	VERIFY_GC(pGC, stuff->gc, client);\
	if ((pGC->depth != pDraw->depth) ||\
	    (pGC->pScreen != pDraw->pScreen))\
	    return (BadMatch);\
	client->lastDrawable = pDraw;\
	client->lastDrawableID = drawID;\
	client->lastGC = pGC;\
	client->lastGCID = stuff->gc;\
    }\
    else\
    {\
        pGC = client->lastGC;\
        pDraw = client->lastDrawable;\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);

#define WriteReplyToClient(pClient, size, pReply) \
   if ((pClient)->swapped) \
      (*ReplySwapVector[((xReq *)(pClient)->requestBuffer)->reqType]) \
           (pClient, (int)(size), pReply); \
      else (void) WriteToClient(pClient, (int)(size), (char *)(pReply));

#define WriteSwappedDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else (void) WriteToClient (pClient, (int)(size), (char *)(pbuf));

typedef struct _TimeStamp *TimeStampPtr;

#ifndef _XTYPEDEF_CLIENTPTR
typedef struct _Client *ClientPtr; /* also in misc.h */
#define _XTYPEDEF_CLIENTPTR
#endif

#ifdef LBX
typedef struct _ClientPublic {
    int             (*writeToClient) ();
    int             (*uncompressedWriteToClient) ();
    unsigned long   (*requestLength) ();
    int             (*readRequest)();
} ClientPublicRec, *ClientPublicPtr;

#define WriteToClient(client,len,buf)   (((client)->public.writeToClient)(client,len,buf))
#define UncompressedWriteToClient(client,len,buf)   (((client)->public.uncompressedWriteToClient)(client,len,buf))
#define ReadRequestFromClient(client)   ((client)->public.readRequest(client))
#define RequestLength(r,client,g,p)           (*(client)->public.requestLength) (r,client,g,p)
#endif /* LBX */

typedef struct _WorkQueue	*WorkQueuePtr;


extern ClientPtr requestingClient;
extern ClientPtr *clients;
extern ClientPtr serverClient;
extern int currentMaxClients;

#ifndef __alpha
typedef long HWEventQueueType;
#else
typedef int HWEventQueueType;
#endif
typedef HWEventQueueType* HWEventQueuePtr;

extern HWEventQueuePtr checkForInput[2];

/* dispatch.c */

extern void SetInputCheck(
#if NeedFunctionPrototypes
    HWEventQueuePtr /*c0*/,
    HWEventQueuePtr /*c1*/
#endif
);

extern void CloseDownClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void UpdateCurrentTime(
#if NeedFunctionPrototypes
    void
#endif
);

extern void UpdateCurrentTimeIf(
#if NeedFunctionPrototypes
    void
#endif
);

extern void InitSelections(
#if NeedFunctionPrototypes
    void
#endif
);

extern void FlushClientCaches(
#if NeedFunctionPrototypes
    XID /*id*/
#endif
);

extern int dixDestroyPixmap(
#if NeedFunctionPrototypes
    pointer /*value*/,
    XID /*pid*/
#endif
);

extern void CloseDownRetainedResources(
#if NeedFunctionPrototypes
    void
#endif
);

extern void InitClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*i*/,
    pointer /*ospriv*/
#endif
);

extern ClientPtr NextAvailableClient(
#if NeedFunctionPrototypes
    pointer /*ospriv*/
#endif
);

extern void SendErrorToClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    unsigned int /*majorCode*/,
    unsigned int /*minorCode*/,
    XID /*resId*/,
    int /*errorCode*/
#endif
);

extern void DeleteWindowFromAnySelections(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void MarkClientException(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

/* dixutils.c */

extern void CopyISOLatin1Lowered(
#if NeedFunctionPrototypes
    unsigned char * /*dest*/,
    unsigned char * /*source*/,
    int /*length*/
#endif
);

extern WindowPtr LookupWindow(
#if NeedFunctionPrototypes
    XID /*rid*/,
    ClientPtr /*client*/
#endif
);

extern pointer LookupDrawable(
#if NeedFunctionPrototypes
    XID /*rid*/,
    ClientPtr /*client*/
#endif
);

extern ClientPtr LookupClient(
#if NeedFunctionPrototypes
    XID /*rid*/
#endif
);

extern void NoopDDA(
#if NeedVarargsPrototypes
    void *,
    ...
#endif
);

extern int AlterSaveSetForClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    WindowPtr /*pWin*/,
    unsigned /*mode*/
#endif
);

extern void DeleteWindowFromAnySaveSet(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void BlockHandler(
#if NeedFunctionPrototypes
    pointer /*pTimeout*/,
    pointer /*pReadmask*/
#endif
);

extern void WakeupHandler(
#if NeedFunctionPrototypes
    int /*result*/,
    pointer /*pReadmask*/
#endif
);

typedef struct timeval ** OSTimePtr;

typedef void (* BlockHandlerProcPtr)(
#if NeedNestedPrototypes
    pointer /* blockData */,
    OSTimePtr /* pTimeout */,
    pointer /* pReadmask */
#endif
);

typedef void (* WakeupHandlerProcPtr)(
#if NeedNestedPrototypes
    pointer /* blockData */,
    int /* result */,
    pointer /* pReadmask */
#endif
);

extern Bool RegisterBlockAndWakeupHandlers(
#if NeedFunctionPrototypes
    BlockHandlerProcPtr /*blockHandler*/,
    WakeupHandlerProcPtr /*wakeupHandler*/,
    pointer /*blockData*/
#endif
);

extern void RemoveBlockAndWakeupHandlers(
#if NeedFunctionPrototypes
    BlockHandlerProcPtr /*blockHandler*/,
    WakeupHandlerProcPtr /*wakeupHandler*/,
    pointer /*blockData*/
#endif
);

extern void InitBlockAndWakeupHandlers(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ProcessWorkQueue(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool QueueWorkProc(
#if NeedFunctionPrototypes
    Bool (* /*function*/)(),
    ClientPtr /*client*/,
    pointer /*closure*/
#endif
);

extern Bool ClientSleep(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Bool (* /*function*/)(),
    pointer /*closure*/
#endif
);

extern Bool ClientSignal(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void ClientWakeup(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern Bool ClientIsAsleep(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

/* atom.c */

extern Atom MakeAtom(
#if NeedFunctionPrototypes
    char * /*string*/,
    unsigned /*len*/,
    Bool /*makeit*/
#endif
);

extern Bool ValidAtom(
#if NeedFunctionPrototypes
    Atom /*atom*/
#endif
);

extern char *NameForAtom(
#if NeedFunctionPrototypes
    Atom /*atom*/
#endif
);

extern void AtomError(
#if NeedFunctionPrototypes
    void
#endif
);

extern void FreeAllAtoms(
#if NeedFunctionPrototypes
    void
#endif
);

extern void InitAtoms(
#if NeedFunctionPrototypes
    void
#endif
);

/* events.c */

extern int DeliverEvents(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    xEventPtr /*xE*/,
    int /*count*/,
    WindowPtr /*otherParent*/
#endif
);

extern void WriteEventsToClient(
#if NeedFunctionPrototypes
    ClientPtr /*pClient*/,
    int	     /*count*/,
    xEventPtr /*events*/
#endif
);

extern int TryClientEvents(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xEventPtr /*pEvents*/,
    int /*count*/,
    Mask /*mask*/,
    Mask /*filter*/,
    GrabPtr /*grab*/
#endif
);

extern int EventSelectForWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    ClientPtr /*client*/,
    Mask /*mask*/
#endif
);

extern int EventSuppressForWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    ClientPtr /*client*/,
    Mask /*mask*/,
    Bool * /*checkOptional*/
#endif
);

extern int MaybeDeliverEventsToClient(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    xEventPtr /*pEvents*/,
    int /*count*/,
    Mask /*filter*/,
    ClientPtr /*dontClient*/
#endif
);

extern void WindowsRestructured(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ResetClientPrivates(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AllocateClientPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool AllocateClientPrivate(
#if NeedFunctionPrototypes
    int /*index*/,
    unsigned /*amount*/
#endif
);

/*
 *  callback manager stuff
 */

#ifndef _XTYPEDEF_CALLBACKLISTPTR
typedef struct _CallbackList *CallbackListPtr; /* also in misc.h */
#define _XTYPEDEF_CALLBACKLISTPTR
#endif

typedef void (*CallbackProcPtr) (
#if NeedNestedPrototypes
    CallbackListPtr *, pointer, pointer
#endif
);

typedef Bool (*AddCallbackProcPtr) (
#if NeedNestedPrototypes
    CallbackListPtr *, CallbackProcPtr, pointer
#endif
);

typedef Bool (*DeleteCallbackProcPtr) (
#if NeedNestedPrototypes
    CallbackListPtr *, CallbackProcPtr, pointer
#endif
);

typedef void (*CallCallbacksProcPtr) (
#if NeedNestedPrototypes
    CallbackListPtr *, pointer
#endif
);

typedef void (*DeleteCallbackListProcPtr) (
#if NeedNestedPrototypes
    CallbackListPtr *
#endif
);

typedef struct _CallbackProcs {
    AddCallbackProcPtr		AddCallback;
    DeleteCallbackProcPtr	DeleteCallback;
    CallCallbacksProcPtr	CallCallbacks;
    DeleteCallbackListProcPtr	DeleteCallbackList;
} CallbackFuncsRec, *CallbackFuncsPtr;

extern Bool CreateCallbackList(
#if NeedFunctionPrototypes
    CallbackListPtr * /*pcbl*/,
    CallbackFuncsPtr /*cbfuncs*/
#endif
);

extern Bool AddCallback(
#if NeedFunctionPrototypes
    CallbackListPtr * /*pcbl*/,
    CallbackProcPtr /*callback*/,
    pointer /*data*/
#endif
);

extern Bool DeleteCallback(
#if NeedFunctionPrototypes
    CallbackListPtr * /*pcbl*/,
    CallbackProcPtr /*callback*/,
    pointer /*data*/
#endif
);

extern void CallCallbacks(
#if NeedFunctionPrototypes
    CallbackListPtr * /*pcbl*/,
    pointer /*call_data*/
#endif
);

extern void DeleteCallbackList(
#if NeedFunctionPrototypes
    CallbackListPtr * /*pcbl*/
#endif
);

extern void InitCallbackManager(
#if NeedFunctionPrototypes
    void
#endif
);

/*
 *  ServerGrabCallback stuff
 */

extern CallbackListPtr ServerGrabCallback;

typedef enum {SERVER_GRABBED, SERVER_UNGRABBED,
	      CLIENT_PERVIOUS, CLIENT_IMPERVIOUS } ServerGrabState;

typedef struct {
    ClientPtr client;
    ServerGrabState grabstate;
} ServerGrabInfoRec;

/*
 *  EventCallback stuff
 */

extern CallbackListPtr EventCallback;

typedef struct {
    ClientPtr client;
    struct _xEvent *events;
    int count;
} EventInfoRec;

#endif /* DIX_H */
