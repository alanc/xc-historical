/* $XConsortium: error.c,v 1.2 93/09/05 21:08:41 mor Exp $ */
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

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>
#include <stdio.h>

void
_IceErrorBadMinor (iceConn, offendingMinor, severity)

IceConn	iceConn;
int	offendingMinor;
int	severity;

{
    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceBadMinor,
	0);

    IceFlush (iceConn);
}


void
_IceErrorBadState (iceConn, offendingMinor, severity)

IceConn	iceConn;
int	offendingMinor;
int	severity;

{
    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceBadState,
	0);

    IceFlush (iceConn);
}


void
_IceErrorBadLength (iceConn, offendingMinor, severity)

IceConn	iceConn;
int	offendingMinor;
int	severity;

{
    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceBadLength,
	0);

    IceFlush (iceConn);
}


void
_IceErrorBadValue (iceConn, offendingMinor, offset, length, value)

IceConn		iceConn;
int		offendingMinor;
int		offset;
int		length;		/* in bytes */
IcePointer	value;

{
    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	IceCanContinue,
	IceBadValue,
	WORD64COUNT (length));

    IceWriteData32 (iceConn, 4, &offset);
    IceWriteData32 (iceConn, 4, &length);
    IceWriteData (iceConn, length, (char *) value);

    if (PAD64 (length))
	IceWritePad (iceConn, PAD64 (length));
    
    IceFlush (iceConn);
}


void
_IceErrorNoAuthentication (iceConn, offendingMinor)

IceConn	iceConn;
int	offendingMinor;

{
    int severity = (offendingMinor == ICE_ConnectionSetup) ?
	IceFatalToConnection : IceFatalToProtocol;

    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceNoAuth,
	0);

    IceFlush (iceConn);
}


void
_IceErrorNoVersion (iceConn, offendingMinor)

IceConn	iceConn;
int	offendingMinor;

{
    int severity = (offendingMinor == ICE_ConnectionSetup) ?
	IceFatalToConnection : IceFatalToProtocol;

    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceNoVersion,
	0);

    IceFlush (iceConn);
}


void
_IceErrorAuthenticationRejected (iceConn, offendingMinor, reason)

IceConn	iceConn;
int	offendingMinor;
char	*reason;

{
    char *pBuf, *pStart;
    int bytes = XPCS_BYTES (reason);

    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	IceFatalToProtocol,
	IceAuthRejected,
	WORD64COUNT (bytes));

    pBuf = pStart = IceAllocScratch (iceConn, PADDED_BYTES64 (bytes));
    STORE_XPCS (pBuf, reason);

    IceWriteData (iceConn, PADDED_BYTES64 (bytes), pStart);
    IceFlush (iceConn);
}


void
_IceErrorAuthenticationFailed (iceConn, offendingMinor, reason)

IceConn	iceConn;
int	offendingMinor;
char	*reason;

{
    char *pBuf, *pStart;
    int bytes = XPCS_BYTES (reason);

    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	IceFatalToProtocol,
	IceAuthFailed,
	WORD64COUNT (bytes));

    pBuf = pStart = IceAllocScratch (iceConn, PADDED_BYTES64 (bytes));
    STORE_XPCS (pBuf, reason);

    IceWriteData (iceConn, PADDED_BYTES64 (bytes), pStart);
    IceFlush (iceConn);
}


void
_IceErrorProtocolDuplicate (iceConn, protocolName)

IceConn	iceConn;
char	*protocolName;

{
    char *pBuf, *pStart;
    int bytes = XPCS_BYTES (protocolName);

    IceErrorHeader (iceConn,
	0, ICE_ProtocolSetup,
	iceConn->sequence - 1,
	IceFatalToProtocol,
	IceProtocolDuplicate,
	WORD64COUNT (bytes));

    pBuf = pStart = IceAllocScratch (iceConn, PADDED_BYTES64 (bytes));
    STORE_XPCS (pBuf, protocolName);

    IceWriteData (iceConn, PADDED_BYTES64 (bytes), pStart);
    IceFlush (iceConn);
}


void
_IceErrorMajorOpcodeDuplicate (iceConn, majorOpcode)

IceConn	iceConn;
int	majorOpcode;

{
    char mOp = (char) majorOpcode;

    IceErrorHeader (iceConn,
	0, ICE_ProtocolSetup,
	iceConn->sequence - 1,
	IceFatalToProtocol,
	IceMajorOpcodeDuplicate,
	1 /* length */);

    IceWriteData (iceConn, 8, &mOp);
    IceFlush (iceConn);
}


void
_IceErrorUnknownProtocol (iceConn, protocolName)

IceConn	iceConn;
char	*protocolName;

{
    char *pBuf, *pStart;
    int bytes = XPCS_BYTES (protocolName);

    IceErrorHeader (iceConn,
	0, ICE_ProtocolSetup,
	iceConn->sequence - 1,
	IceFatalToProtocol,
	IceUnknownProtocol,
	WORD64COUNT (bytes));

    pBuf = pStart = IceAllocScratch (iceConn, PADDED_BYTES64 (bytes));
    STORE_XPCS (pBuf, protocolName);

    IceWriteData (iceConn, PADDED_BYTES64 (bytes), pStart);
    IceFlush (iceConn);
}


void
_IceErrorBadMajor (iceConn, offendingMajor, offendingMinor, severity)

IceConn	iceConn;
int     offendingMajor;
int     offendingMinor;
int	severity;

{
    char maj = (char) offendingMajor;

    IceErrorHeader (iceConn,
	0, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceBadMajor,
	1 /* length */);

    IceWriteData (iceConn, 8, &maj);
    IceFlush (iceConn);
}



/*
 * Default error handler.
 */

void
_IceDefaultErrorHandler (iceConn, offendingMinorOpcode, offendingSequence,
    errorClass, severity, data)

IceConn		iceConn;
int		offendingMinorOpcode;
unsigned long	offendingSequence;
int 		errorClass;
int		severity;
IcePointer	data;

{
    char *str;
    char *pData = (char *) data;

    switch (offendingMinorOpcode)
    {
        case ICE_ConnectionSetup:
            str = "ConnectionSetup";
	    break;
        case ICE_AuthRequired:
            str = "AuthRequired";
	    break;
        case ICE_AuthReply:
            str = "AuthReply";
	    break;
        case ICE_AuthNextPhase:
            str = "AuthNextPhase";
	    break;
        case ICE_ConnectionReply:
            str = "ConnectionReply";
	    break;
        case ICE_ProtocolSetup:
            str = "ProtocolSetup";
	    break;
        case ICE_ProtocolReply:
            str = "ProtocolReply";
	    break;
        case ICE_Ping:
            str = "Ping";
	    break;
        case ICE_PingReply:
            str = "PingReply";
	    break;
        case ICE_WantToClose:
            str = "WantToClose";
	    break;
        case ICE_NoClose:
            str = "NoClose";
	    break;
	default:
	    str = "";
	}

    fprintf (stderr, "ICE error:  Offending minor opcode    = %d (%s)\n",
	offendingMinorOpcode, str);

    fprintf (stderr, "            Offending sequence number = %d\n",
	offendingSequence);

    switch (errorClass)
    {
        case IceBadMinor:
            str = "BadMinor";
            break;
        case IceBadState:
            str = "BadState";
            break;
        case IceBadLength:
            str = "BadLength";
            break;
        case IceBadValue:
            str = "BadValue";
            break;
        case IceBadMajor:
            str = "BadMajor";
            break;
        case IceNoAuth:
            str = "NoAuthentication";
            break;
        case IceNoVersion:
            str = "NoVersion";
            break;
        case IceAuthRejected:
            str = "AuthenticationRejected";
            break;
        case IceAuthFailed:
            str = "AuthenticationFailed";
            break;
        case IceProtocolDuplicate:
            str = "ProtocolDuplicate";
            break;
        case IceMajorOpcodeDuplicate:
            str = "MajorOpcodeDuplicate";
            break;
        case IceUnknownProtocol:
            str = "UnknownProtocol";
            break;
	default:
	    str = "???";
    }

    fprintf (stderr, "            Error class               = %s\n", str);

    if (severity == IceCanContinue)
	str = "CanContinue";
    else if (severity == IceFatalToProtocol)
	str = "FatalToProtocol";
    else if (severity == IceFatalToConnection)
	str = "FatalToConnection";
    else
	str = "???";

    fprintf (stderr, "            Severity                  = %s\n", str);

    switch (errorClass)
    {
        case IceBadValue:
            break;

        case IceBadMajor:

	    fprintf (stderr, "Major opcode : %d\n", (int) *pData);
            break;

        case IceAuthRejected:

	    EXTRACT_XPCS (pData, str);
	    fprintf (stderr, "Reason : %s\n", str);
            break;

        case IceAuthFailed:

	    EXTRACT_XPCS (pData, str);
	    fprintf (stderr, "Reason : %s\n", str);
            break;

        case IceProtocolDuplicate:

	    EXTRACT_XPCS (pData, str);
	    fprintf (stderr, "Protocol name : %s\n", str);
            break;

        case IceMajorOpcodeDuplicate:

	    fprintf (stderr, "Major opcode : %d\n", (int) *pData);
            break;

        case IceUnknownProtocol:

	    EXTRACT_XPCS (pData, str);
	    fprintf (stderr, "Protocol name : %s\n", str);
            break;

	default:
	    break;
    }

    if (severity == IceFatalToProtocol || severity == IceFatalToConnection)
	exit (1);
}



/* 
 * This procedure sets the ICE error handler to be the specified
 * routine.  If NULL is passed in the default error handler is restored.
 * The function's return value is the previous error handler.
 */
 
IceErrorHandler
IceSetErrorHandler (handler)

IceErrorHandler handler;

{
    IceErrorHandler oldHandler = _IceErrorHandler;

    if (handler != NULL)
	_IceErrorHandler = handler;
    else
	_IceErrorHandler = _IceDefaultErrorHandler;

    return (oldHandler);
}



/*
 * Default IO error handler.
 */

void
_IceDefaultIOErrorHandler (iceConn)

IceConn iceConn;

{
    fprintf (stderr, "ICE Fatal IO error!  Did an exit().\n");

    exit (1);
}



/* 
 * This procedure sets the ICE fatal I/O error handler to be the
 * specified routine.  If NULL is passed in the default error
 * handler is restored.   The function's return value is the
 * previous error handler.
 */
 
IceIOErrorHandler
IceSetIOErrorHandler (handler)

IceIOErrorHandler handler;

{
    IceIOErrorHandler oldHandler = _IceIOErrorHandler;

    if (handler != NULL)
	_IceIOErrorHandler = handler;
    else
	_IceIOErrorHandler = _IceDefaultIOErrorHandler;

    return (oldHandler);
}
