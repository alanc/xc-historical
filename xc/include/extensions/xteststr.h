/* $XConsortium: xteststr.h,v 1.0 91/04/16 15:03:32 rws Exp $ */
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

#define X_XTestGetVersion	0
#define X_XTestCompareCursor	1

#define XTestNumberEvents	0

#define XTestNumberErrors	0

#define XTEST_MAJOR	1
#define XTEST_MINOR	0

#define XTestCurrentCursor ((Cursor)1)

#define XTESTNAME "XTEST"

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestGetVersion */
    CARD16	length B16;
    CARD8	major;
    CARD8	pad;
    CARD16	minor B16;
} xXTestGetVersionReq;
#define sz_xXTestGetVersionReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	major;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	minor B16;
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
