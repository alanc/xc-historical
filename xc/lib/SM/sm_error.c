/* $XConsortium$ */
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

#include <X11/SM/SMlib.h>
#include <X11/SM/SMlibint.h>
#include <stdio.h>



/*
 * Default Smc error handler.
 */

void
_SmcDefaultErrorHandler (smcConn,
    offendingMinorOpcode, offendingSequence,
    errorClass, severity, data)

SmcConn		smcConn;
int 		offendingMinorOpcode;
unsigned long 	offendingSequence;
int 		errorClass;
int 		severity;
SmPointer 	data;

{
    char *str;
    char *pData = (char *) data;

    switch (offendingMinorOpcode)
    {
        case SM_RegisterClient:
            str = "RegisterClient";
	    break;
        case SM_InteractRequest:
            str = "InteractRequest";
	    break;
        case SM_InteractDone:
            str = "InteractDone";
	    break;
        case SM_SaveYourselfDone:
            str = "SaveYourselfDone";
	    break;
        case SM_CloseConnection:
            str = "CloseConnection";
	    break;
        case SM_SetProperties:
            str = "SetProperties";
	    break;
        case SM_GetProperties:
            str = "GetProperties";
	    break;
	default:
	    str = "";
	}

    fprintf (stderr, "XSMP error:  Offending minor opcode    = %d (%s)\n",
	offendingMinorOpcode, str);

    fprintf (stderr, "             Offending sequence number = %d\n",
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

	default:
	    break;
    }
}



/*
 * Default Sms error handler.
 */

void
_SmsDefaultErrorHandler (smsConn,
    offendingMinorOpcode, offendingSequence,
    errorClass, severity, data)

SmsConn		smsConn;
int 		offendingMinorOpcode;
unsigned long 	offendingSequence;
int 		errorClass;
int 		severity;
SmPointer 	data;

{
    char *str;
    char *pData = (char *) data;

    switch (offendingMinorOpcode)
    {
        case SM_SaveYourself:
            str = "SaveYourself";
	    break;
        case SM_Interact:
            str = "Interact";
	    break;
        case SM_Die:
            str = "Die";
	    break;
        case SM_ShutdownCancelled:
            str = "ShutdownCancelled";
	    break;
	default:
	    str = "";
	}

    fprintf (stderr, "XSMP error:  Offending minor opcode    = %d (%s)\n",
	offendingMinorOpcode, str);

    fprintf (stderr, "             Offending sequence number = %d\n",
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

	default:
	    break;
    }
}



/* 
 * This procedure sets the Smc error handler to be the specified
 * routine.  If NULL is passed in the default error handler is restored.
 * The function's return value is the previous error handler.
 */
 
SmcErrorHandler
SmcSetErrorHandler (handler)

SmcErrorHandler handler;

{
    SmcErrorHandler oldHandler = _SmcErrorHandler;

    if (handler != NULL)
	_SmcErrorHandler = handler;
    else
	_SmcErrorHandler = _SmcDefaultErrorHandler;

    return (oldHandler);
}



/* 
 * This procedure sets the Sms error handler to be the specified
 * routine.  If NULL is passed in the default error handler is restored.
 * The function's return value is the previous error handler.
 */
 
SmsErrorHandler
SmsSetErrorHandler (handler)

SmsErrorHandler handler;

{
    SmsErrorHandler oldHandler = _SmsErrorHandler;

    if (handler != NULL)
	_SmsErrorHandler = handler;
    else
	_SmsErrorHandler = _SmsDefaultErrorHandler;

    return (oldHandler);
}



void
_SmcErrorBadState (iceConn, offendingMinor, severity)

IceConn	iceConn;
int	offendingMinor;
int	severity;

{
    IceErrorHeader (iceConn,
	_SmcOpcode, offendingMinor,
	iceConn->sequence - 1,
	severity,
	IceBadState,
	0);

    IceFlush (iceConn);
}
