/* $XConsortium: ICElibint.h,v 1.2 93/08/20 10:13:59 mor Exp $ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

#ifndef ICELIBINT_H
#define ICELIBINT_H

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICEproto.h>


/*
 * Vendor & Release
 */

#define IceVendorString  "MIT"
#define IceReleaseString "1.0"


/*
 * Pad to a 64 bit boundary
 */

#define PAD64(_bytes) ((8 - ((int) (_bytes) % 8)) % 8)

#define PADDED_BYTES64(_bytes) (_bytes + PAD64 (_bytes))


/*
 * Pad to 32 bit boundary
 */

#define PAD32(_bytes) ((4 - ((int) (_bytes) % 4)) % 4)

#define PADDED_BYTES32(_bytes) (_bytes + PAD32 (_bytes))


/*
 * Number of 8 byte units in _bytes.
 */

#define WORD64COUNT(_bytes) (((unsigned int) ((_bytes) + 7)) >> 3)


/*
 * Number of 4 byte units in _bytes.
 */

#define WORD32COUNT(_bytes) (((unsigned int) ((_bytes) + 3)) >> 2)


/*
 * Given a string, compute the number of bytes for the XPCS representation
 */

#define XPCS_BYTES(_string) \
    (2 + strlen (_string) + PAD32 (2 + strlen (_string)))


/*
 * Size of ICE input/output buffers
 */

#define ICE_INBUFSIZE 1024

#define ICE_OUTBUFSIZE 1024


/*
 * Some internal data structures for processing ICE messages.
 */

typedef Bool (*_IceProcessCoreMsgCB) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* opcode */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */
#endif
);

typedef struct {
    int 			major_version;
    int 			minor_version;
    _IceProcessCoreMsgCB	process_core_msg_cb;
} _IceVersion;


/*
 * Macros for writing messages.
 */

#define IceFlush(_iceConn) \
{ \
    _IceWrite (_iceConn, \
	 _iceConn->outbufptr - _iceConn->outbuf, _iceConn->outbuf); \
    _iceConn->outbufptr = _iceConn->outbuf; \
}

#define IceGetOutBufSize(_iceConn) (_iceConn->outbufmax - _iceConn->outbuf)

#define IceGetHeader(_iceConn, _major, _minor, _headerSize, _msgType, _pMsg) \
    if ((_iceConn->outbufptr + _headerSize) > _iceConn->outbufmax) \
        IceFlush (_iceConn); \
    _pMsg = (_msgType *) _iceConn->outbufptr; \
    _pMsg->majorOpcode = _major; \
    _pMsg->minorOpcode = _minor; \
    _pMsg->length = (_headerSize - SIZEOF (iceMsg)) >> 3; \
    _iceConn->outbufptr += _headerSize; \
    _iceConn->sequence++

#define IceGetHeaderExtra(_iceConn, _major, _minor, _headerSize, _extra, _msgType, _pMsg, _pData) \
    if ((_iceConn->outbufptr + \
	_headerSize + ((_extra) << 3)) > _iceConn->outbufmax) \
        IceFlush (_iceConn); \
    _pMsg = (_msgType *) _iceConn->outbufptr; \
    if ((_iceConn->outbufptr + \
	_headerSize + ((_extra) << 3)) <= _iceConn->outbufmax) \
        _pData = (char *) _pMsg + _headerSize; \
    else \
        _pData = NULL; \
    _pMsg->majorOpcode = _major; \
    _pMsg->minorOpcode = _minor; \
    _pMsg->length = ((_headerSize - SIZEOF (iceMsg)) >> 3) + (_extra); \
    _iceConn->outbufptr += (_headerSize + ((_extra) << 3)); \
    _iceConn->sequence++

#define IceSimpleMessage(_iceConn, _major, _minor) \
{ \
    iceMsg *_pMsg; \
    IceGetHeader (_iceConn, _major, _minor, SIZEOF (iceMsg), iceMsg, _pMsg); \
}

#define IceErrorHeader(_iceConn, _offendingMajorOpcode, _offendingMinorOpcode, _offendingSequenceNum, _severity, _errorClass, _dataLength) \
{ \
    iceErrorMsg	*_pMsg; \
\
    IceGetHeader (_iceConn, _offendingMajorOpcode, ICE_Error, \
	SIZEOF (iceErrorMsg), iceErrorMsg, _pMsg); \
    _pMsg->length += (_dataLength); \
    _pMsg->offendingMinorOpcode = _offendingMinorOpcode; \
    _pMsg->severity = _severity; \
    _pMsg->offendingSequenceNum = _offendingSequenceNum; \
    _pMsg->errorClass = _errorClass; \
}


/*
 * Write data into the ICE output buffer.
 */

#define IceWriteData(_iceConn, _bytes, _data) \
{ \
    if ((_iceConn->outbufptr + (_bytes)) > _iceConn->outbufmax) \
    { \
	IceFlush (_iceConn); \
        _IceWrite (_iceConn, _bytes, _data); \
    } \
    else \
    { \
        bcopy (_data, _iceConn->outbufptr, _bytes); \
        _iceConn->outbufptr += (_bytes); \
    } \
}

#ifndef WORD64

#define IceWriteData16(_iceConn, _bytes, _data) \
    IceWriteData (_iceConn, _bytes, (char *) _data)

#define IceWriteData32(_iceConn, _bytes, _data) \
    IceWriteData (_iceConn, _bytes, (char *) _data)

#endif


/*
 * The IceSendData family of macros bypass copying the data to the
 * ICE connection buffer and send the data directly.  If necessary,
 * the ICE connection buffer is first flushed.
 */

#define IceSendData(_iceConn, _bytes, _data) \
{ \
    if (_iceConn->outbufptr > _iceConn->outbuf) \
	IceFlush (_iceConn); \
    _IceWrite (_iceConn, _bytes, _data); \
}

#ifndef WORD64

#define IceSendData16(_iceConn, _bytes, _data) \
    IceSendData (_iceConn, _bytes, (char *) _data)

#define IceSendData32(_iceConn, _bytes, _data) \
    IceSendData (_iceConn, _bytes, (char *) _data)

#endif


/*
 * Write pad bytes.  Used to force 32 or 64 bit alignment.
 * A maxium of 7 pad bytes can be specified.
 */

#define IceWritePad(_iceConn, _bytes) \
{ \
    if ((_iceConn->outbufptr + (_bytes)) > _iceConn->outbufmax) \
    { \
        char _dummy[7]; \
	IceFlush (_iceConn); \
        _IceWrite (_iceConn, _bytes, _dummy); \
    } \
    else \
    { \
        _iceConn->outbufptr += (_bytes); \
    } \
}


/*
 * Macros for reading messages.
 */

#define IceGetInBufSize(_iceConn) (_iceConn->inbufmax - _iceConn->inbuf)

#define IceReadCompleteMessage(_iceConn, _headerSize, _msgType, _pMsg, _pData)\
{ \
    iceMsg *header = (iceMsg *) (_iceConn->inbuf); \
    int bytes = header->length << 3; \
    if ((IceGetInBufSize (_iceConn) - SIZEOF (iceMsg)) < bytes) \
    { \
        char *temp = _iceConn->inbuf; \
        _iceConn->inbuf = (char *) malloc (bytes + SIZEOF (iceMsg)); \
        bcopy (temp, _iceConn->inbuf, SIZEOF (iceMsg)); \
       	_iceConn->inbufptr = _iceConn->inbuf + SIZEOF (iceMsg); \
        _iceConn->inbufmax = _iceConn->inbuf + bytes + SIZEOF (iceMsg); \
        free (temp); \
    } \
    _pMsg = (_msgType *) (_iceConn->inbuf); \
    _pData = _iceConn->inbuf + _headerSize; \
    _IceRead (_iceConn, bytes, _iceConn->inbufptr); \
    _iceConn->inbufptr += bytes; \
}

#define IceReadSimpleMessage(_iceConn, _msgType, _pMsg) \
    _pMsg = (_msgType *) (_iceConn->inbuf);

#define IceReadMessageHeader(_iceConn, _headerSize, _msgType, _pMsg) \
{ \
    _IceRead (_iceConn, \
	(_headerSize - SIZEOF (iceMsg)), _iceConn->inbufptr); \
    _pMsg = (_msgType *) (_iceConn->inbuf); \
    _iceConn->inbufptr += (_headerSize - SIZEOF (iceMsg)); \
}

#define IceReadData (_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, _bytes, (char *) _pData); \

#ifndef WORD64

#define IceReadData16 (_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, _bytes, (char *) _pData); \

#define IceReadData32 (_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, _bytes, (char *) _pData); \

#endif


/*
 * Read pad bytes (for 32 or 64 bit alignment).
 * A maxium of 7 pad bytes can be specified.
 */

#define IceReadPad (_iceConn, _bytes) \
{ \
    char _dummy[7]; \
    _IceRead (_iceConn, _bytes, _dummy); \
}


/*
 * STORE FOO
 */

#define STORE_CARD8(_pBuf, _val) \
{ \
    *((CARD8 *) _pBuf) = _val; \
    _pBuf += 1; \
}

#ifndef WORD64

#define STORE_CARD16(_pBuf, _val) \
{ \
    *((CARD16 *) _pBuf) = _val; \
    _pBuf += 2; \
}

#define STORE_CARD32(_pBuf, _val) \
{ \
    *((CARD32 *) _pBuf) = _val; \
    _pBuf += 4; \
}

#else /* WORD64 */

#define STORE_CARD16(_pBuf, _val) \
{ \
    struct { \
        int value   :16; \
        int pad     :16; \
    } _d; \
    _d.value = _val; \
    bcopy ((char *) &_d, _pBuf, 2); \
    _pBuf += 2; \
}

#define STORE_CARD32(_pBuf, _val) \
{ \
    struct { \
        int value   :32; \
    } _d; \
    _d.value = _val; \
    bcopy ((char *) &_d, _pBuf, 4); \
    _pBuf += 4; \
}

#endif /* WORD64 */

#define STORE_XPCS(_pBuf, _string) \
{ \
    CARD16 _len = strlen (_string); \
    STORE_CARD16 (_pBuf, _len); \
    bcopy (_string, _pBuf, _len); \
    _pBuf += _len; \
    if (PAD32 (2 + _len)) \
        _pBuf += PAD32 (2 + _len); \
}


/*
 * EXTRACT FOO
 */

#define EXTRACT_CARD8(_pBuf, _val) \
{ \
    _val = *((CARD8 *) _pBuf); \
    _pBuf += 1; \
}

#ifndef WORD64

#define EXTRACT_CARD16(_pBuf, _val) \
{ \
    _val = *((CARD16 *) _pBuf); \
    _pBuf += 2; \
}

#define EXTRACT_CARD32(_pBuf, _val) \
{ \
    _val = *((CARD32 *) _pBuf); \
    _pBuf += 4; \
}

#else /* WORD64 */

#define EXTRACT_CARD16(_pBuf, _val) \
{ \
    _val = *(_pBuf + 0) & 0xff; 	/* 0xff incase _pBuf is signed */ \
    _val <<= 8; \
    _val |= *(_pBuf + 1) & 0xff;\
    _pBuf += 2; \
}

#define EXTRACT_CARD32(_pBuf, _val) \
{ \
    _val = *(_pBuf + 0) & 0xff; 	/* 0xff incase _pBuf is signed */ \
    _val <<= 8; \
    _val |= *(_pBuf + 1) & 0xff;\
    _val <<= 8; \
    _val |= *(_pBuf + 2) & 0xff;\
    _val <<= 8; \
    _val |= *(_pBuf + 3) & 0xff;\
    _pBuf += 4; \
}

#endif /* WORD64 */

#define EXTRACT_XPCS(_pBuf, _string) \
{ \
    CARD16 _len; \
    EXTRACT_CARD16 (_pBuf, _len); \
    _string = (char *) malloc (_len + 1); \
    bcopy (_pBuf, _string, _len); \
    _pBuf += _len; \
    _string[_len] = '\0'; \
    if (PAD32 (2 + _len)) \
        _pBuf += PAD32 (2 + _len); \
}

#define EXTRACT_LISTOF_XPCS(_pBuf, _count, _strings) \
{ \
    int _i; \
    for (_i = 0; _i < _count; _i++) \
        EXTRACT_XPCS (_pBuf, _strings[_i]); \
}


/*
 * ICE replies (not processed via callbacks because we block)
 */

#define ICE_CONNECTION_REPLY	1
#define ICE_CONNECTION_ERROR	2
#define ICE_PROTOCOL_REPLY	3
#define ICE_PROTOCOL_ERROR	4

typedef struct {
    int		  type;
    int 	  version_index;
    char	  *vendor;
    char          *release;
} _IceConnectionReply;

typedef struct {
    int		  type;
    char	  *error_message;
} _IceConnectionError;

typedef struct {
    int		  type;
    int 	  major_opcode;
    int		  version_index;
    char	  *vendor;
    char	  *release;
} _IceProtocolReply;

typedef struct {
    int		  type;
    char	  *error_message;
} _IceProtocolError;


typedef union {
    int			type;
    _IceConnectionReply	connection_reply;
    _IceConnectionError	connection_error;
    _IceProtocolReply	protocol_reply;
    _IceProtocolError	protocol_error;
} _IceReply;


/*
 * Extern declarations
 */

extern _IceProtocol	_IceProtocols[];
extern int         	_IceLastMajorOpcode;

extern int		_IceAuthCount;
extern IceOCLauthRec	_IceOCLauthRecs[];
extern IceACLauthRec	_IceACLauthRecs[];

extern int		_IceVersionCount;
extern _IceVersion	_IceVersions[];

extern IceErrorHandler   _IceErrorHandler;
extern IceIOErrorHandler _IceIOErrorHandler;


extern void
_IceErrorBadMinor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void
_IceErrorBadState (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void
_IceErrorBadLength (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void
_IceErrorBadValue (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    int			/* offset */,
    int			/* length */,
    IcePointer		/* value */
#endif
);

extern void
_IceErrorBadMajor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMajor */,
    int			/* offendingMinor */
#endif
);

extern void
_IceErrorNoAuthentication (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */
#endif
);

extern void
_IceErrorNoVersion (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */
#endif
);

extern void
_IceErrorAuthenticationRejected (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    char *		/* reason */
#endif
);

extern void
_IceErrorAuthenticationFailed (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    char *		/* reason */
#endif
);

extern void
_IceErrorProtocolDuplicate (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    char *		/* protocolName */
#endif
);

extern void
_IceErrorMajorOpcodeDuplicate (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

extern void
_IceErrorUnknownProtocol (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    char *		/* protocolName */
#endif
);

extern void
_IceRead (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* nbytes */,
    char *		/* ptr */
#endif
);

extern void
_IceWrite (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* nbytes */,
    char *		/* ptr */
#endif
);

extern void
_IceAddOpcodeMapping (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* hisOpcode */,
    int			/* myOpcode */
#endif
);

extern void
_IceFreeConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#endif /* ICELIBINT */