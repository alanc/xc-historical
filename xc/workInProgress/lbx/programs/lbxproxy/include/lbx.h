/* $XConsortium: lbx.h,v 1.5 95/05/24 16:11:18 mor Exp $ */
/*
 * $NCDOr$
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */

#ifndef _LBX_H_
#define _LBX_H_
#include "misc.h"
#include "os.h"
#include "util.h"
#define _XLBX_SERVER_
#include "lbxstr.h"

typedef struct _XServer *XServerPtr;

/* cache stuff */
#define MAX_ATOM_LENGTH 256	/* screw any large ones */
#define MAX_COLORNAME_LENGTH    256	/* screw any large ones */


typedef struct proptagdata {
    Atom        type;
    int         format;
    int         length;		/* in bytes */
    pointer     data;
}           PropertyTagDataRec, *PropertyTagDataPtr;


typedef struct _querytag {
    XID         tag;
    int         tagtype;
    union {
	struct _query_font {
	    int         compression;
	}           query_font;
	struct _modmap {
	    int         keyspermod;
	}           modmap;
	struct _keymap {
	    int         keyspercode;
	    int         count;
	    KeyCode     first;
	}           keymap;
	struct _getprop {
	    CARD32      offset;
	    CARD32      length;
	    PropertyTagDataRec ptd;
	}           getprop;
	struct _setup {
	    int         majorVersion;
	    int         minorVersion;
	    pointer     changes;
	}           setup;
    }           typedata;
}           QueryTagRec, *QueryTagPtr;

typedef struct _replystuff {
    int         sequenceNumber;	/* expected sequence number */
    int         request;	/* request code */
    int         extension;	/* extension opcode */
    Bool        lbx_req;	/* is this an LBX request? */
    Bool        guessed;	/* did we have to guess? */
    union {			/* reply data */
	struct atom_data {
	    Atom        atom;
	    int         len;
	    char        str[MAX_ATOM_LENGTH];
	}           lbxatom;
	struct colorname {
	    char        name[MAX_COLORNAME_LENGTH];
	    int         namelen;
	    Colormap    cmap;
	}           lbxlookupcolor;
	struct alloccolor {
	    Colormap    cmap;
	    int         red,
	                green,
	                blue;
	}           lbxalloccolor;
	struct allocnamedcolor {
	    char        name[MAX_COLORNAME_LENGTH];
	    int         namelen;
	    Colormap    cmap;
	}           lbxallocnamedcolor;
	struct querytag {
	    QueryTagRec info;
	}           lbxquerytag;
	struct getkeymap {
	    int         count;
	    int         first;
	}           lbxgetkeymap;
	struct getprop {
	    CARD32      offset;
	    CARD32      length;
	}           lbxgetprop;
	struct changeprop {
	    PropertyTagDataRec ptd;
	}           lbxchangeprop;
	struct queryextension {
	    char       *name;
	}           lbxqueryextension;
	struct getimage {
#ifdef LBX_STATS
	    Drawable	drawable;
	    INT16	x;
	    INT16	y;
#endif
	    CARD16      width;
	    CARD16      height;
	}           lbxgetimage;
	pointer     lbxextensiondata;
    }           request_info;
    struct _replystuff *next;
}           ReplyStuffRec, *ReplyStuffPtr;

typedef struct _replydata {
    int         dlen;
    xReply      reply;
    pointer     data;
    int         delay_seq_no;	/* what seq i'm waiting for */
    struct _replydata *next;
}           ReplyDataRec, *ReplyDataPtr;

typedef struct _ClientPublic {
    int             (*writeToClient) ();
    int             (*uncompressedWriteToClient) ();
    unsigned long   (*requestLength) ();
    int             (*readRequest)();
} ClientPublicRec, *ClientPublicPtr;

typedef struct _Client {
    int         index;
    Mask        clientAsMask;
    pointer     requestBuffer;
    pointer     osPrivate;	/* for OS layer, including scheduler */
    Bool        swapped;
    void        (*pSwapReplyFunc) (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		int		/* size */,
		void *		/* pbuf */
#endif
);
    XID         errorValue;
    int         sequence;
    int         closeDownMode;
    int         clientGone;
    int         noClientException;	/* this client died or needs to be
					 * killed */
    XServerPtr server;
    Drawable    serverID;
    GContext    lastGCID;
    pointer    *saveSet;
    int         awaitingSetup;
    int         (**requestVector) (
#if NeedNestedPrototypes
		ClientPtr /* pClient */
#endif
);
    CARD32	req_len;		/* length of current request */
    Bool	big_requests;		/* supports large requests */

#ifdef DEBUG
#define MAX_REQUEST_LOG 100
    unsigned char requestLog[MAX_REQUEST_LOG];
    int         requestLogIndex;
#endif

    ClientPublicRec public;
    int         lbxIndex;

    Bool        cache_safe;	/* safe to short circuit request? */
    Bool        can_delay_reply;/* safe to delay short-circuit request? */

    int         sequenceNumber;	/* what we've seen */
    int         sequence_lost;	/* requests short-circuited */
    int         lastReply;	/* last sequence number seen from server */

    int         yanked;		/* total requests short-circuited */

    KeyCode     minKeyCode,
                maxKeyCode;

    CARD8          imageByteOrder;        /* LSBFirst, MSBFirst */
    CARD8          bitmapBitOrder;        /* LeastSignificant, MostSign...*/
    CARD8          bitmapScanlineUnit,     /* 8, 16, 32 */
                   bitmapScanlinePad;     /* 8, 16, 32 */

    ReplyStuffPtr replylist;	/* list of pending replies */
    ReplyDataPtr replydata;	/* list of delayed short-circuited replies */
    Drawable	drawableCache[GFX_CACHE_SIZE];
    GContext	gcontextCache[GFX_CACHE_SIZE];

    int		server_client_index; /* We need to keep track of the client */
				     /* index used by server.  The 'index' */
				     /* field of ClientRec is the index used */
				     /* by the proxy. */
}           ClientRec;

#define WriteToClient(client,len,buf)   (((client)->public.writeToClient)(client,len,buf))
#define ReadRequestFromClient(client)   ((client)->public.readRequest(client))
#define RequestLength(r,client,g,p)           (*(client)->public.requestLength) (r,client,g,p)

#define	LBXSequenceNumber(client)	(client->sequenceNumber)
#define	LBXCacheSafe(client)	(client->cache_safe)
#define	LBXCanDelayReply(client) (client->can_delay_reply)
#define	LBXSequenceLost(client)	(client->sequence_lost)
#define	LBXYanked(client)	(client->yanked)
#define	LBXLastReply(client)	(client->lastReply)
#define	LBXReplyList(client)	(client->replylist)
#define	LBXReplyData(client)	(client->replydata)
#define	LBXMinKeyCode(client)	(client->minKeyCode)
#define	LBXMaxKeyCode(client)	(client->maxKeyCode)
#define	LBXImageByteOrder(client) (client->imageByteOrder)
#define	LBXBitmapBitOrder(client) (client->bitmapBitOrder)
#define	LBXBitmapScanlineUnit(client) (client->bitmapScanlineUnit)
#define	LBXBitmapScanlinePad(client) (client->bitmapScanlinePad)
#define	LBXDrawableCache(client)(client->drawableCache)
#define	LBXGContextCache(client)(client->gcontextCache)

extern ClientPtr *clients;
#define MAXCLIENTS       128
#define NullClient ((ClientPtr) 0)
extern ClientPtr serverClient;
extern int currentMaxClients;

extern ClientPtr lastLbxClientIndexLookup; /* see di/resource.c for comments */


/*
 * Notes on request short-circuiting
 *
 * X guarentees that any replies, events or errors generated by a
 * previous request will be sent before those of a later request.
 * this means that any requests that can be handled by the proxy
 * must have their reply sent after any previous events or errors.
 *
 * there are 3 possible stages implemented here:
 *
 * - fully correct protocol, where nothing can be out of order
 * - mostly correct protocol, where only errors can be out of order
 * - poor protocol, where events & errors can be out of order
 *
 * for most applications, poor is 'good enough', because they don't
 * depend on events coming in order, and instead let Xlib do
 * the work.
 *
 * depending on the type of protocol being used, the cache_safe
 * flag in the ClientRec will be set appropriately, and if TRUE,
 * the proxy is cleared to short-circuit a request.
 *
 * for full correct protocol, the flag is set if the previous request
 * was a round trip and the reply has come back.
 *
 * for a mostly correct protocol, the flag is set for the above
 * case, and for requests that can generate only errors (mostly
 * graphics)
 *
 * for a 'good enough' protocol, the flag is always on, and we let
 * the client sort things out.
 *
 * (note than in no case do we allow replies to come out of order.)
 *
 *
 * Note on Xlib's handling of sequences:
 *
 * Xlib usually waits for the reply to sync request before
 * issuing another -- its archetecture depends on it.  However,
 * this is not required.  Xlib itself misses this in at least one
 * case (XGetWindowAttributes, which uses both the GetWindowAttributes
 * and GetGeometry requests).
 *
 * The upshot is that the proxy has to be able to handle seeing
 * another request before it receives an expected reply.
 *
 * this means we can't assume a single pending reply, but many.
 * currently these get tacked onto a list.
 *
 *
 * Notes on how the proxy attempts to solve all of this:
 *
 * There are 4 request that it will attempt to short-circuit:
 * InternAtom, GetAtomName, LookupColor, AllocColor, and AllocNamedColor.
 * All the code to do this is in lbxfuncs.c, and all of it works in
 * essentially the same way -- whenever possible, it will try to respond 
 * to the client without sending the request all the way to the X server.
 * The goal is to remove a roudn-trip entirely -- even though not a lot of data
 * is involved, remving the roundtrip can make this useful.

 * Unfortunately, figuring out *when* its safe is very nasty.  First
 * there are all the possibile stages outlined above.  If the proxy
 * isn't waiting, thinking there might be pending data, it can always
 * short-circuit the request.  However, this is the rare case.  When
 * the proxy can't immediately respond, it attempts to short-circuit anyways,
 * delaying the reply until whatever it was waiting on arrives.  Often
 * it can't -- if the most paranoid type of checking is turned on, it'll
 * never work.  The complex part occurs with loose checking.  If it knows
 * a reply is coming back, it can delay the short-circuit until it gets
 * the reply.  This works fine for the core protocol, but breaks down 
 * again for extensions -- to be safe, any unknown extensions are assumed
 * to have replies -- but since they may never arrive, the delayed
 * short-circuit can never be flushed.  To avoid this, whether a reply
 * has been guessed at is tracked, and this info is used to decide
 * whether a short-circuit can be delayed -- if not, it always needs
 * to do the full round-trip.
 *
 * The complexity really makes me wonder if this stuff is worth the effort...
 */


/* short-circuiting protocol */

/* #define	PROTOCOL_RAW		pure Xremote */
/* #define	PROTOCOL_SLOW		no caching at all */
/* #define	PROTOCOL_FULL		best w/ accuracy */
/* #define	PROTOCOL_MOST		cheat on errors */
#define	PROTOCOL_POOR		/* cheat on errors & events */

#undef	DEBUG

#undef	TRACE

/* possible states of request handlers.  some requests are short-circuited
 * (yanked), some are replaced with LBX requests, and some are let through
 * unchanged
 *
 * any replaced request expecting a reply needs to be sure the sequence
 * number isn't lagging before its sends its data
 */
#define	REQ_NOCHANGE	0	/* pass through untouched -- lbx sends data */
#define	REQ_YANK	1	/* completely remove from stream */
#define	REQ_REPLACE	2	/* replace with new encoding */
#define	REQ_PASSTHROUGH	3	/* pass thorugh untouched, other sends data */

/*
 * how far we can let the server's sequence number slide before
 * bringing it up to date
 *
 * NOTE -- command only supports a char's worth of data
 */
#define	MAX_SEQUENCE_LOST	255

/*
 * size of motion event queue
 *
 * this specifies the maximum number of motion events that the server
 * can spew out at once.  the proxy will send it NUM_MOTION_EVENTS
 * AllowMotion requests at startup, and then send a new one each time it
 * processes a Motion event.  this will prevent the wire from being
 * swamped by MotionNotify.
 */

/* XXX this should be a calculation based on the wire latency */

#define	NUM_MOTION_EVENTS	8


#ifdef LBX_STATS
extern int  intern_good,
            intern_miss;
extern int  getatom_good,
            getatom_miss;
extern int  luc_good,
            luc_miss;
extern int  ac_good,
            ac_miss;
extern int  anc_good,
            anc_miss;

extern int  getmodmap_tag,	/* tag only */
            getmodmap_full;
extern int  getkeymap_tag,	/* tag only */
            getkeymap_full;
extern int  queryfont_tag,	/* tag only */
            queryfont_full;
extern int  getsetup_tag,	/* tag only */
            getsetup_full;

extern int  getprop_tag,
            getprop_full;

extern int  missed_tag;
extern int  tag_bytes_unsent;	/* approx data kept off wire by tags */


extern void DumpStats();
extern void ZeroStats();

#endif /* LBX_STATS */

/* lbxfuncs.c */

extern void FinishSetupReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*setup_len*/,
    xConnSetup * /*setup_data*/,
    pointer /*changes*/,
    int /*majorVer*/,
    int /*minorVer*/
#endif
);

extern void FinishModmapReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqnum*/,
    int /*kpm*/,
    pointer /*data*/
#endif
);

extern void FinishKeymapReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqnum*/,
    int /*kpk*/,
    int /*first*/,
    int /*count*/,
    char * /*data*/
#endif
);

extern void FinishQueryFontReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqnum*/,
    int /*length*/,
    pointer /*data*/
#endif
);

extern int UnsquishFontInfo(
#if NeedFunctionPrototypes
    int	/*compression*/,
    xLbxFontInfo * /*fdata*/,
    int /*dlen*/,
    pointer * /*qfr*/
#endif
);

extern int ProcLBXInternAtom(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXGetAtomName(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXCreateColormap(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXAllocColor(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXAllocNamedColor(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXFreeColormap(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXCopyColormapAndFree(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXFreeColors(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXLookupColor(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXGetModifierMapping(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXGetKeyboardMapping(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcLBXQueryFont(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int MakeLBXReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int FinishLBXRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*yank*/
#endif
);

extern Bool DoLBXReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/,
    int /*len*/
#endif
);

/* dispatch.c */

extern int Dispatch(
#if NeedFunctionPrototypes
    void
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

extern ClientPtr NextAvailableClient(
#if NeedFunctionPrototypes
    pointer /*ospriv*/
#endif
);

extern int ProcInitialConnection(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcEstablishConnection(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void CloseDownClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcStandardRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ProcBadRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void AdjustProcVector(
#if NeedFunctionPrototypes
    void
#endif
);

/* lbxutil.c */

extern ReplyStuffPtr NewReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void RemoveReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    ReplyStuffPtr /*rp*/
#endif
);

extern ReplyStuffPtr GetReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern ReplyStuffPtr GetMatchingReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqno*/
#endif
);

extern int NumReplies(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int NumGuessedReplies(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int LargestReplySeqNo(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern Bool SaveReplyData(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xReply * /*rep*/,
    int /*len*/,
    pointer /*data*/
#endif
);

extern Bool FlushDelayedReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqno*/
#endif
);

extern void BumpSequence(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void ForceSequenceUpdate(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void WriteToClientPad(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*len*/,
    char * /*data*/
#endif
);

extern void QueryTag(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    QueryTagPtr /*qtp*/
#endif
);

extern int GetQueryTagReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/
#endif
);

extern void LbxFreeTag(
#if NeedFunctionPrototypes
    XID /*tag*/,
    int /*tagtype*/
#endif
);

extern void LbxSendTagData(
#if NeedFunctionPrototypes
    XID /*tag*/,
    int /*tagtype*/
#endif
);

extern void SendInitLBXPackets(
#if NeedFunctionPrototypes
    void
#endif
);

extern void GetConnectionInfo(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xConnSetup * /*cs*/,
    CARD32 * /*changes*/
#endif
);

extern void LbxCleanupSession(
#if NeedFunctionPrototypes
    void
#endif
);

/* unsquish.c */

extern int EventLength(
#if NeedFunctionPrototypes
    xEvent * /*ev*/
#endif
);

extern Bool UnsquishEvent(
#if NeedFunctionPrototypes
    xReply * /*rep*/,
    xEvent * /*ev*/,
    int * /*lenp*/
#endif
);

/* props.c */

extern int ProcLBXChangeProperty(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern Bool GetLbxChangePropertyReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/
#endif
);

extern void FinishGetPropertyReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*seqnum*/,
    CARD32 /*offset*/,
    CARD32 /*length*/,
    PropertyTagDataPtr /*ptdp*/,
    pointer /*pdata*/
#endif
);

extern Bool GetLbxGetPropertyReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/
#endif
);

extern int ProcLBXGetProperty(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

/* init.c */

extern int InitMux(
#if NeedFunctionPrototypes
    char * /*dpy_name*/,
    int * /*requestp*/,
    int * /*eventp*/,
    int * /*errorp*/,
    int * /*sequencep*/
#endif
);

/* gfx.c */

extern Bool GetLbxImageReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/
#endif
);

#endif
