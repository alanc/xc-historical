/* $XConsortium: sm_error.c,v 1.8 94/03/16 15:51:02 mor Exp $ */
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

#include <X11/SM/SMlib.h>
#include "SMlibint.h"
#include <stdio.h>



/*
 * Default Smc error handler.
 */

void
_SmcDefaultErrorHandler (smcConn, swap,
    offendingMinorOpcode, offendingSequence,
    errorClass, severity, values)

SmcConn		smcConn;
Bool		swap;
int 		offendingMinorOpcode;
unsigned long 	offendingSequence;
int 		errorClass;
int 		severity;
SmPointer 	values;

{
    char *pData = (char *) values;
    char *str;

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

    fprintf (stderr, "\n");

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

    fprintf (stderr, "             Error class               = %s\n", str);

    if (severity == IceCanContinue)
	str = "CanContinue";
    else if (severity == IceFatalToProtocol)
	str = "FatalToProtocol";
    else if (severity == IceFatalToConnection)
	str = "FatalToConnection";
    else
	str = "???";

    fprintf (stderr, "             Severity                  = %s\n", str);

    switch (errorClass)
    {
        case IceBadValue:
        {
	    int offset, length, val;

	    EXTRACT_CARD32 (pData, swap, offset);
	    EXTRACT_CARD32 (pData, swap, length);

	    fprintf (stderr,
		"             BadValue Offset           = %d\n", offset);
	    fprintf (stderr,
		"             BadValue Length           = %d\n", length);

	    if (length <= 4)
	    {
		if (length == 1)
		    val = (int) *pData;
		else if (length == 2)
		{
		    EXTRACT_CARD16 (pData, swap, val);
		}
		else
		{
		    EXTRACT_CARD32 (pData, swap, val);
		}

		fprintf (stderr,
	            "             BadValue                  = %d\n", val);
	    }
            break;
	}

	default:
	    break;
    }

    fprintf (stderr, "\n");
    exit (1);
}



/*
 * Default Sms error handler.
 */

void
_SmsDefaultErrorHandler (smsConn, swap,
    offendingMinorOpcode, offendingSequence,
    errorClass, severity, values)

SmsConn		smsConn;
Bool		swap;
int 		offendingMinorOpcode;
unsigned long 	offendingSequence;
int 		errorClass;
int 		severity;
SmPointer 	values;

{
    char *pData = (char *) values;
    char *str;

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

    fprintf (stderr, "\n");

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

    fprintf (stderr, "             Error class               = %s\n", str);

    if (severity == IceCanContinue)
	str = "CanContinue";
    else if (severity == IceFatalToProtocol)
	str = "FatalToProtocol";
    else if (severity == IceFatalToConnection)
	str = "FatalToConnection";
    else
	str = "???";

    fprintf (stderr, "             Severity                  = %s\n", str);

    switch (errorClass)
    {
        case IceBadValue:
        {
	    int offset, length, val;

	    EXTRACT_CARD32 (pData, swap, offset);
	    EXTRACT_CARD32 (pData, swap, length);

	    fprintf (stderr,
		"             BadValue Offset           = %d\n", offset);
	    fprintf (stderr,
		"             BadValue Length           = %d\n", length);

	    if (length <= 4)
	    {
		if (length == 1)
		    val = (int) *pData;
		else if (length == 2)
		{
		    EXTRACT_CARD16 (pData, swap, val);
		}
		else
		{
		    EXTRACT_CARD32 (pData, swap, val);
		}

		fprintf (stderr,
	            "             BadValue                  = %d\n", val);
	    }
            break;
	}

	default:
	    break;
    }

    fprintf (stderr, "\n\n");

    /* don't exit() - that would kill the SM - pretty devastating */
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
