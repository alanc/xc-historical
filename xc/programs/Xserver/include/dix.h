/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
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
/* $XConsortium: dix.h,v 1.70 93/11/18 20:35:52 rob Exp $ */

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

/* MTX changes the way reply structures are declared to minimize #ifdef's */
#ifdef MTX

#define REPLY_DECL(_type,_name)				\
	PooledMessagePtr msg;				\
	_type *_name = GetReplyMessage(_type, &msg)

#define MTX_REP_CHECK_RETURN(_name,_error_value)	\
	if (!_name)					\
		return _error_value

#else /* MTX */

#define REPLY_DECL(_type,_name)				\
	_type _reply_buf_, *_name = &_reply_buf_

#define MTX_REP_CHECK_RETURN(_name,_error_value)	/* nothing */

#endif /* MTX */

/* MTX cannot use local memory for data of global message pool */
#ifdef MTX
#define MTX_LOCAL_ALLOC xalloc
#define MTX_LOCAL_FREE xfree
#else /* MTX */
#define MTX_LOCAL_ALLOC ALLOCATE_LOCAL
#define MTX_LOCAL_FREE DEALLOCATE_LOCAL
#endif /* MTX */

#ifdef MTX
/* MTX assumes routines that call WriteReplyToClient have local variable msg */
#define WriteReplyToClient(pClient, size, pReply) \
    SendReplyToClient(pClient, msg);
#else
#define WriteReplyToClient(pClient, size, pReply) \
   if ((pClient)->swapped) \
      (*ReplySwapVector[((xReq *)(pClient)->requestBuffer)->reqType]) \
           (pClient, (int)(size), pReply); \
      else (void) WriteToClient(pClient, (int)(size), (char *)(pReply));
#endif

#define WriteSwappedDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else (void) WriteToClient (pClient, (int)(size), (char *)(pbuf));

typedef struct _TimeStamp *TimeStampPtr;

#ifndef _XTYPEDEF_CLIENTPTR
typedef struct _Client *ClientPtr; /* also in misc.h */
#define _XTYPEDEF_CLIENTPTR
#endif

typedef struct _WorkQueue	*WorkQueuePtr;


extern ClientPtr requestingClient;
extern ClientPtr *clients;
extern ClientPtr serverClient;
extern int currentMaxClients;
#ifdef __alpha /* XXX temporary until driver is fixed */
extern int *checkForInput[2];
#else
extern long *checkForInput[2];
#endif

/* dispatch.c */

extern void SetInputCheck(
#if NeedFunctionPrototypes
    long* /*c0*/,
    long* /*c1*/
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
#endif /* DIX_H */
