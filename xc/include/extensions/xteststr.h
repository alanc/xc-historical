/* $XConsortium: xteststr.h,v 1.7 93/02/02 14:36:15 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#ifndef _XTESTSTR_H_
#define _XTESTSTR_H_

#define Window CARD32
#define Time CARD32
#define Cursor CARD32

#define XTestCurrentCursor ((Cursor)1)

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestGetVersion */
    CARD16	length B16;
    CARD8	majorVersion;
    CARD8	pad;
    CARD16	minorVersion B16;
} xXTestGetVersionReq;
#define sz_xXTestGetVersionReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	majorVersion;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	minorVersion B16;
    CARD16	pad0 B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXTestGetVersionReply;
#define sz_xXTestGetVersionReply 32

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestCompareCursor */
    CARD16	length B16;
    Window	window B32;
    Cursor	cursor B32;
} xXTestCompareCursorReq;
#define sz_xXTestCompareCursorReq 12

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	same;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXTestCompareCursorReply;
#define sz_xXTestCompareCursorReply 32

/* used only on the client side */
typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestFakeInput */
    CARD16	length B16;
    BYTE	type;
    BYTE	detail;
    CARD16	pad0 B16;
    Time	time B32;
    Window	root B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    INT16	rootX B16, rootY B16;
    CARD32	pad3 B32;
    CARD16	pad4 B16;
    CARD8	pad5;
    CARD8	deviceid;
} xXTestFakeInputReq;
#define sz_xXTestFakeInputReq 36

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestGrabControl */
    CARD16	length B16;
    BOOL	impervious;
    CARD8	pad0;
    CARD8	pad1;
    CARD8	pad2;
} xXTestGrabControlReq;
#define sz_xXTestGrabControlReq 8

#undef Window
#undef Time
#undef Cursor

#endif /* _XTESTSTR_H_ */
