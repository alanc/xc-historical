/* $XConsortium: ICElibint.h,v 1.28 94/01/31 10:54:05 mor Exp $ */
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

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef ICELIBINT_H
#define ICELIBINT_H

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICEproto.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *malloc();
#endif

#ifndef NULL
#define NULL 0
#endif


/*
 * Vendor & Release
 */

#define IceVendorString  "MIT"
#define IceReleaseString "1.0"


/*
 * Pad to a 64 bit boundary
 */

#define PAD64(_bytes) ((8 - ((unsigned int) (_bytes) % 8)) % 8)

#define PADDED_BYTES64(_bytes) (_bytes + PAD64 (_bytes))


/*
 * Pad to 32 bit boundary
 */

#define PAD32(_bytes) ((4 - ((unsigned int) (_bytes) % 4)) % 4)

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
 * Given a string, compute the number of bytes for the STRING representation
 */

#define STRING_BYTES(_string) \
    (2 + strlen (_string) + PAD32 (2 + strlen (_string)))


/*
 * Size of ICE input/output buffers
 */

#define ICE_INBUFSIZE 1024

#define ICE_OUTBUFSIZE 1024



/*
 * Some internal data structures
 */

typedef struct _IceSavedReplyWait {
    IceReplyWaitInfo		*reply_wait;
    Bool			reply_ready;
    struct _IceSavedReplyWait	*next;
} _IceSavedReplyWait;

typedef struct _IcePingWait {
    IcePingReplyProc		ping_reply_proc;
    IcePointer			client_data;
    struct _IcePingWait 	*next;
} _IcePingWait;

typedef struct {
    char		*vendor;
    char		*release;
    int			version_count;
    IcePoVersionRec	*version_recs;
    int			auth_count;
    char		**auth_names;
    IcePoAuthProc	*auth_procs;
    IceIOErrorProc	io_error_proc;
} _IcePoProtocol;

typedef struct {
    char			*vendor;
    char			*release;
    int				version_count;
    IcePaVersionRec		*version_recs;
    IceProtocolSetupNotifyProc	protocol_setup_notify_proc;
    int				auth_count;
    char			**auth_names;
    IcePaAuthProc		*auth_procs;
    IceHostBasedAuthProc	host_based_auth_proc;
    IceIOErrorProc		io_error_proc;
} _IcePaProtocol;

typedef struct {
    char		*protocol_name;
    _IcePoProtocol	*orig_client;
    _IcePaProtocol   	*accept_client;
} _IceProtocol;

typedef struct {
    Bool			in_use;
    int				my_opcode;
    _IceProtocol		*protocol;
    Bool			accept_flag;
    union {
	IcePaProcessMsgProc	accept_client;
	IcePoProcessMsgProc	orig_client;
    } process_msg_proc;
} _IceProcessMsgInfo;

typedef struct {
    int		his_version_index;
    int		my_version_index;
    char	*his_vendor;
    char	*his_release;
    char	my_auth_index;
    IcePointer 	my_auth_state;
    Bool	must_authenticate;
} _IceConnectToMeInfo;

typedef struct {
    int		his_opcode;
    int		my_opcode;
    int		his_version_index;
    int		my_version_index;
    char	*his_vendor;
    char	*his_release;
    char	my_auth_index;
    IcePointer 	my_auth_state;
    Bool	must_authenticate;
} _IceProtoSetupToMeInfo;

typedef struct {
    Bool 	auth_active;
    char	my_auth_index;
    IcePointer 	my_auth_state;
} _IceConnectToYouInfo;

typedef struct {
    int		my_opcode;
    int		my_auth_count;
    int		*my_auth_indices;
    Bool 	auth_active;
    char	my_auth_index;
    IcePointer	my_auth_state;
} _IceProtoSetupToYouInfo;


/*
 * ICE listen object
 */

struct _IceListenObj {
    struct _XtransConnInfo 	*trans_conn; /* transport connection object */
    char			*network_id;
    IceHostBasedAuthProc 	host_based_auth_proc;
};


/*
 * ICE connection object
 */

struct _IceConn {

    IceConnectStatus connection_status; /* pending, accepted, rejected */

    char waiting_for_byteorder; 	/* waiting for a ByteOrder message? */
    char swap;  		        /* do we need to swap on reads? */
    unsigned char my_ice_version_index; /* which version are we using? */

    struct _XtransConnInfo *trans_conn; /* transport connection object */
    unsigned long sequence;     	/* Sequence number of last message */

    char *connection_string;		/* network connection string */
    char *vendor;			/* other client's vendor */
    char *release;			/* other client's release */

    char *inbuf;			/* Input buffer starting address */
    char *inbufptr;			/* Input buffer index pointer */
    char *inbufmax;			/* Input buffer maximum+1 address */

    char *outbuf;			/* Output buffer starting address */
    char *outbufptr;			/* Output buffer index pointer */
    char *outbufmax;			/* Output buffer maximum+1 address */

    char *scratch;			/* scratch buffer */
    unsigned long scratch_size;		/* scratch size */


    /*
     * Before we read a message, the major opcode of the message must be
     * mapped to our corresponding major opcode (the two clients can use
     * different opcodes for the same protocol).  In order to save space,
     * we keep track of the mininum and maximum major opcodes used by the
     * other client.  To get the information on how to process this message,
     * we do the following...
     *
     * processMsgInfo = iceConn->process_msg_info[
     *     message->majorOpcode - iceConn->his_min_opcode]
     *
     * Note that the number of elements in the iceConn->process_msg_info
     * array is always (iceConn->his_max_opcode - iceConn->his_min_opcode + 1).
     * We check process_msg_info->in_use to see if the opcode is being used.
     */

    _IceProcessMsgInfo		*process_msg_info;
    char 			his_min_opcode;   /* [1..255] */
    char			his_max_opcode;	  /* [1..255] */


    /*
     * Number of times this iceConn was returned in IceOpenConnection
     * or IceAcceptConnection.
     */

    unsigned char		open_ref_count;


    /*
     * Number of active protocols.
     */

    unsigned char		proto_ref_count;


    /*
     * If this ICE connection was created with IceAcceptConnection,
     * the listen_obj field is set to the listen object.  Otherwise,
     * the listen_obj field is NULL.
     */

    IceListenObj		listen_obj;


    /*
     * If we know the connection was terminated by the other side, we
     * want to skip sending a WantToClose in IceCloseConnection.
     */

    char			skip_want_to_close;


    /*
     * Did we send a WantToClose message?  This will get cancelled if we
     * receive a NoClose or a ProtocolSetup.  If this is the case, the
     * other side will be responsible for sending a WantToClose.
     */

    char			want_to_close;


    /*
     * We need to keep track of all the replies we're waiting for.
     * Check the comments in process.c for how this works.
     */

    _IceSavedReplyWait		*saved_reply_waits;


    /*
     * We keep track of all Pings sent from the client.  When the Ping reply
     * arrives, we remove it from the list.
     */

    _IcePingWait		*ping_waits;


    /*
     * Some state for a client doing a Connection/Protocol Setup
     */

    _IceConnectToYouInfo	*connect_to_you;
    _IceProtoSetupToYouInfo	*protosetup_to_you;


    /*
     * Some state for a client receiving a Connection/Protocol Setup
     */

    _IceConnectToMeInfo		*connect_to_me;
    _IceProtoSetupToMeInfo	*protosetup_to_me;

};


/*
 * Some internal data structures for processing ICE messages.
 */

typedef Bool (*_IceProcessCoreMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */
#endif
);

typedef struct {
    int 			major_version;
    int 			minor_version;
    _IceProcessCoreMsgProc	process_core_msg_proc;
} _IceVersion;


/*
 * Macros for writing messages.
 */

#define IceFlush(_iceConn) \
{ \
    _IceWrite (_iceConn, \
	(unsigned long) (_iceConn->outbufptr - _iceConn->outbuf), \
	_iceConn->outbuf); \
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
        _IceWrite (_iceConn, (unsigned long) (_bytes), _data); \
    } \
    else \
    { \
        memcpy (_iceConn->outbufptr, _data, _bytes); \
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
    _IceWrite (_iceConn, (unsigned long) (_bytes), _data); \
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
        _IceWrite (_iceConn, (unsigned long) (_bytes), _dummy); \
    } \
    else \
    { \
        _iceConn->outbufptr += (_bytes); \
    } \
}


/*
 * Macros for reading messages.
 */

#define IceReadCompleteMessage(_iceConn, _headerSize, _msgType, _pMsg, _pData)\
{ \
    unsigned long _bytes; \
    IceReadMessageHeader (_iceConn, _headerSize, _msgType, _pMsg); \
    _bytes = (_pMsg->length << 3) - (_headerSize - SIZEOF (iceMsg)); \
    if ((_iceConn->inbufmax - _iceConn->inbufptr) >= _bytes) \
    { \
	_IceRead (_iceConn, _bytes, _iceConn->inbufptr); \
	_pData = _iceConn->inbufptr; \
	_iceConn->inbufptr += _bytes; \
    } \
    else \
    { \
	_pData = (char *) malloc ((unsigned) _bytes); \
        if (_pData) \
	    _IceRead (_iceConn, _bytes, _pData); \
        else \
	    _IceReadSkip (_iceConn, _bytes); \
    } \
}

#define IceDisposeCompleteMessage(_iceConn, _pData) \
    if ((char *) _pData < _iceConn->inbuf || \
	(char *) _pData >= _iceConn->inbufmax) \
        free ((char *) _pData);


#define IceReadSimpleMessage(_iceConn, _msgType, _pMsg) \
    _pMsg = (_msgType *) (_iceConn->inbuf);

#define IceReadMessageHeader(_iceConn, _headerSize, _msgType, _pMsg) \
{ \
    _IceRead (_iceConn, \
	(unsigned long) (_headerSize - SIZEOF (iceMsg)), \
	_iceConn->inbufptr); \
    _pMsg = (_msgType *) (_iceConn->inbuf); \
    _iceConn->inbufptr += (_headerSize - SIZEOF (iceMsg)); \
}

#define IceReadData(_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, (unsigned long) (_bytes), (char *) _pData); \

#ifndef WORD64

#define IceReadData16(_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, (unsigned long) (_bytes), (char *) _pData); \

#define IceReadData32(_iceConn, _bytes, _pData) \
    _IceRead (_iceConn, (unsigned long) (_bytes), (char *) _pData); \

#endif


/*
 * Read pad bytes (for 32 or 64 bit alignment).
 * A maxium of 7 pad bytes can be specified.
 */

#define IceReadPad(_iceConn, _bytes) \
{ \
    char _dummy[7]; \
    _IceRead (_iceConn, (unsigned long) (_bytes), _dummy); \
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
    memcpy (_pBuf, &_d, 2); \
    _pBuf += 2; \
}

#define STORE_CARD32(_pBuf, _val) \
{ \
    struct { \
        int value   :32; \
    } _d; \
    _d.value = _val; \
    memcpy (_pBuf, &_d, 4); \
    _pBuf += 4; \
}

#endif /* WORD64 */

#define STORE_STRING(_pBuf, _string) \
{ \
    CARD16 _len = strlen (_string); \
    STORE_CARD16 (_pBuf, _len); \
    memcpy (_pBuf, _string, _len); \
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

#define EXTRACT_CARD16(_pBuf, _swap, _val) \
{ \
    _val = *((CARD16 *) _pBuf); \
    _pBuf += 2; \
    if (_swap) \
        _val = lswaps (_val); \
}

#define EXTRACT_CARD32(_pBuf, _swap, _val) \
{ \
    _val = *((CARD32 *) _pBuf); \
    _pBuf += 4; \
    if (_swap) \
        _val = lswapl (_val); \
}

#else /* WORD64 */

#define EXTRACT_CARD16(_pBuf, _swap, _val) \
{ \
    _val = *(_pBuf + 0) & 0xff; 	/* 0xff incase _pBuf is signed */ \
    _val <<= 8; \
    _val |= *(_pBuf + 1) & 0xff;\
    _pBuf += 2; \
    if (_swap) \
        _val = lswaps (_val); \
}

#define EXTRACT_CARD32(_pBuf, _swap, _val) \
{ \
    _val = *(_pBuf + 0) & 0xff; 	/* 0xff incase _pBuf is signed */ \
    _val <<= 8; \
    _val |= *(_pBuf + 1) & 0xff;\
    _val <<= 8; \
    _val |= *(_pBuf + 2) & 0xff;\
    _val <<= 8; \
    _val |= *(_pBuf + 3) & 0xff;\
    _pBuf += 4; \
    if (_swap) \
        _val = lswapl (_val); \
}

#endif /* WORD64 */

#define EXTRACT_STRING(_pBuf, _swap, _string) \
{ \
    CARD16 _len; \
    EXTRACT_CARD16 (_pBuf, _swap, _len); \
    _string = (char *) malloc (_len + 1); \
    memcpy (_string, _pBuf, _len); \
    _pBuf += _len; \
    _string[_len] = '\0'; \
    if (PAD32 (2 + _len)) \
        _pBuf += PAD32 (2 + _len); \
}

#define EXTRACT_LISTOF_STRING(_pBuf, _swap, _count, _strings) \
{ \
    int _i; \
    for (_i = 0; _i < _count; _i++) \
        EXTRACT_STRING (_pBuf, _swap, _strings[_i]); \
}



/*
 * Byte swapping
 */

/* byte swap a long literal */
#define lswapl(_val) ((((_val) & 0xff) << 24) |\
		   (((_val) & 0xff00) << 8) |\
		   (((_val) & 0xff0000) >> 8) |\
		   (((_val) >> 24) & 0xff))

/* byte swap a short literal */
#define lswaps(_val) ((((_val) & 0xff) << 8) | (((_val) >> 8) & 0xff))



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
 * Watch for ICE connection create/destroy.
 */

typedef struct _IceWatchedConnection {
    IceConn				iceConn;
    IcePointer				watch_data;
    struct _IceWatchedConnection	*next;
} _IceWatchedConnection;

typedef struct _IceWatchProc {
    IceWatchProc		watch_proc;
    IcePointer			client_data;
    _IceWatchedConnection	*watched_connections;
    struct _IceWatchProc	*next;
} _IceWatchProc;


/*
 * Locking
 */

#define LockIceConn(_iceConn)
#define UnlockIceConn(_iceConn)


/*
 * Extern declarations
 */

extern IceConn     	_IceConnectionObjs[];
extern char	    	*_IceConnectionStrings[];
extern int     		_IceConnectionCount;

extern _IceProtocol	_IceProtocols[];
extern int         	_IceLastMajorOpcode;

extern int		_IceAuthCount;
extern char		*_IceAuthNames[];
extern IcePoAuthProc	_IcePoAuthProcs[];
extern IcePaAuthProc	_IcePaAuthProcs[];

extern int		_IcePaAuthDataEntryCount;
extern IceAuthDataEntry _IcePaAuthDataEntries[];

extern int		_IceVersionCount;
extern _IceVersion	_IceVersions[];

extern _IceWatchProc	*_IceWatchProcs;

extern IceErrorHandler   _IceErrorHandler;
extern IceIOErrorHandler _IceIOErrorHandler;


extern void _IceErrorBadMinor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void _IceErrorBadState (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void _IceErrorBadLength (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void _IceErrorBadValue (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */,
    int			/* offendingMinor */,
    int			/* offset */,
    int			/* length */,
    IcePointer		/* value */
#endif
);

extern void _IceErrorBadMajor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMajor */,
    int			/* offendingMinor */,
    int			/* severity */
#endif
);

extern void _IceErrorNoAuthentication (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */
#endif
);

extern void _IceErrorNoVersion (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */
#endif
);

extern void _IceErrorAuthenticationRejected (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    char *		/* reason */
#endif
);

extern void _IceErrorAuthenticationFailed (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* offendingMinor */,
    char *		/* reason */
#endif
);

extern void _IceErrorProtocolDuplicate (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    char *		/* protocolName */
#endif
);

extern void _IceErrorMajorOpcodeDuplicate (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

extern void _IceErrorUnknownProtocol (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    char *		/* protocolName */
#endif
);

extern Status _IceRead (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    unsigned long	/* nbytes */,
    char *		/* ptr */
#endif
);

extern void _IceReadSkip (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    unsigned long	/* nbytes */
#endif
);

extern void _IceWrite (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    unsigned long	/* nbytes */,
    char *		/* ptr */
#endif
);

extern void _IceAddOpcodeMapping (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* hisOpcode */,
    int			/* myOpcode */
#endif
);

extern char *_IceGetPeerName (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern void _IceFreeConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    Bool		/* ignoreWatchProcs */
#endif
);

extern void _IceAddReplyWait (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */
#endif
);

extern IceReplyWaitInfo *_IceSearchReplyWaits (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

extern void _IceSetReplyReady (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */
#endif
);

extern Bool _IceCheckReplyReady (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */
#endif
);

extern void _IceConnectionOpened (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern void _IceConnectionClosed (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern void _IceGetPoValidAuthIndices (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    int			/* num_auth_names */,
    char **		/* auth_names */,
    int	*		/* num_indices_ret */,
    int	*		/* indices_ret */
#endif
);

extern void _IceGetPaValidAuthIndices (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    int			/* num_auth_names */,
    char **		/* auth_names */,
    int	*		/* num_indices_ret */,
    int	*		/* indices_ret */
#endif
);

#endif /* ICELIBINT */
