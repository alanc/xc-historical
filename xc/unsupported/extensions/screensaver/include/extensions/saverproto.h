/*
 * $XConsortium: saverproto.h,v 1.1 92/02/13 16:08:42 keith Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _SAVERPROTO_H_
#define _SAVERPROTO_H_
#include "saver.h"

#define X_ScreenSaverQueryVersion   0

typedef struct _ScreenSaverQueryVersion {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverQueryVersion */
    CARD16 length B16;
    CARD8 clientMajor;
    CARD8 clientMinor;
    CARD16 unused B16;	
} xScreenSaverQueryVersionReq;
#define sz_xScreenSaverQueryVersionReq	8

typedef struct {
    CARD8 type;			/* X_Reply */
    CARD8 unused;			/* not used */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 majorVersion B16;	/* major version of protocol */
    CARD16 minorVersion B16;	/* minor version of protocol */
    CARD32 pad0 B32;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
} xScreenSaverQueryVersionReply;
#define sz_xScreenSaverQueryVersionReply	32

#define X_ScreenSaverQueryInfo   1

typedef struct _ScreenSaverQueryInfo {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverQueryInfo */
    CARD16 length B16;
    Drawable drawable B32;
} xScreenSaverQueryInfoReq;
#define sz_xScreenSaverQueryInfoReq	8

typedef struct {
    CARD8 type;			/* X_Reply */
    BYTE state;			/* Off, On */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    Window window B32;
    Time tilOrSince B32;
    Time idle B32;
    CARD32 eventMask B32;
    BYTE kind;			/* Blanked, Internal, External */
    CARD8 pad0;
    CARD16 pad1 B16;
    CARD32 pad2 B32;
} xScreenSaverQueryInfoReply;
#define sz_xScreenSaverQueryInfoReply	32

#define X_ScreenSaverSelectInput   2

typedef struct _ScreenSaverSelectInput {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverSelectInput */
    CARD16 length B16;
    Drawable drawable B32;
    CARD32 eventMask B32;
} xScreenSaverSelectInputReq;
#define sz_xScreenSaverSelectInputReq	12

#define X_ScreenSaverSetAttributes   3

typedef struct _ScreenSaverSetAttributes {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverSetAttributes */
    CARD16 length B16;
    Drawable drawable B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16, borderWidth B16;
    BYTE c_class;
    CARD8 depth;
    VisualID visualID B32;
    CARD32 mask B32;
} xScreenSaverSetAttributesReq;
#define sz_xScreenSaverSetAttributesReq	28

#define X_ScreenSaverUnsetAttributes   4

typedef struct _ScreenSaverUnsetAttributes {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverUnsetAttributes */
    CARD16 length B16;
    Drawable drawable B32;
} xScreenSaverUnsetAttributesReq;
#define sz_xScreenSaverUnsetAttributesReq	8

typedef struct _ScreenSaverNotify {
    CARD8 type;			/* always eventBase + ScreenSaverNotify */
    BYTE state;			/* off, on, cycle */
    CARD16 sequenceNumber B16;
    Time timestamp B32;
    Window root B32;
    Window window B32;		/* screen saver window */
    BYTE kind;			/* blanked, internal, external */
    BYTE forced;
    CARD16 pad0 B16;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
} xScreenSaverNotifyEvent;
#define sz_xScreenSaverNotifyEvent	32

#endif /* _SAVERPROTO_H_ */
