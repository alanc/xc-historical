/* $XConsortium: bigreqstr.h,v 1.1 92/09/07 13:37:31 rws Exp $ */
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

#define X_BigReqGetVersion	0
#define X_BigReqControl		1
#define X_BigReqQueryState	2

#define XBigReqNumberEvents	0

#define XBigReqNumberErrors	0

#define XBigReqMajorVersion	1
#define XBigReqMinorVersion	0

#define XBigReqExtensionName	"BIG-REQUESTS"

typedef struct {
    CARD8	reqType;	/* always XBigReqCode */
    CARD8	brReqType;	/* always X_BigReqGetVersion */
    CARD16	length B16;
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
} xBigReqGetVersionReq;
#define sz_xBigReqGetVersionReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xBigReqGetVersionReply;
#define sz_xBigReqGetVersionReply 32

typedef struct {
    CARD8	reqType;	/* always XBigReqCode */
    CARD8	brReqType;	/* always X_BigReqControl */
    CARD16	length B16;
    BOOL	enable;
    CARD8	pad0;
    CARD8	pad1;
    CARD8	pad2;
} xBigReqControlReq;
#define sz_xBigReqControlReq 8

typedef struct {
    CARD8	reqType;	/* always XBigReqCode */
    CARD8	brReqType;	/* always X_BigReqQueryState */
    CARD16	length B16;
} xBigReqQueryStateReq;
#define sz_xBigReqQueryStateReq 4

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	enabled;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	max_request_size B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xBigReqQueryStateReply;
#define sz_xBigReqQueryStateReply 32
