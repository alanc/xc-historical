
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
/* $XConsortium: dixstruct.h,v 1.19 93/09/03 08:28:57 dpw Exp $ */

#ifndef DIXSTRUCT_H
#define DIXSTRUCT_H

#include "dix.h"
#include "resource.h"
#include "cursor.h"
#include "gc.h"
#include "pixmap.h"
#include "X11/Xmd.h"

/*
 * 	direct-mapped hash table, used by resource manager to store
 *      translation from client ids to server addresses.
 */

typedef struct _TimeStamp {
    unsigned long months;	/* really ~49.7 days */
    unsigned long milliseconds;
}           TimeStamp;

#ifdef DEBUG
#define MAX_REQUEST_LOG 100
#endif

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
    DrawablePtr lastDrawable;
    Drawable    lastDrawableID;
    GCPtr       lastGC;
    GContext    lastGCID;
    pointer    *saveSet;
    int         numSaved;
    pointer     screenPrivate[MAXSCREENS];
    int         (**requestVector) (
#if NeedNestedPrototypes
		ClientPtr /* pClient */
#endif
);
    unsigned long req_len;		/* length of current request */
    Bool	big_requests;		/* supports large requests */
    int		priority;

#ifdef DEBUG
    unsigned char requestLog[MAX_REQUEST_LOG];
    int         requestLogIndex;
#endif
}           ClientRec;

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		pointer		/* closure */
#endif
);
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;

extern TimeStamp currentTime;
extern TimeStamp lastDeviceEventTime;

extern TimeStamp ClientTimeToServerTime(
#if NeedFunctionPrototypes
    CARD32 /*c*/
#endif
);

#endif				/* DIXSTRUCT_H */
