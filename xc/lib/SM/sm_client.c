/* $XConsortium: sm_client.c,v 1.13 93/11/08 11:27:24 mor Exp $ */
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
#include "globals.h"
#include <locale.h>


SmcConn
SmcOpenConnection (networkIdsList, callbacks,
    previousId, clientIdRet, errorLength, errorStringRet)

char 		*networkIdsList;
SmcCallbacks	*callbacks;
char 		*previousId;
char 		**clientIdRet;
int  		errorLength;
char 		*errorStringRet;

{
    SmcConn			smcConn;
    IceConn			iceConn;
    char 			*ids;
    IceProtocolSetupStatus	setupstat;
    int				majorVersion;
    int				minorVersion;
    char			*vendor = NULL;
    char			*release = NULL;
    smRegisterClientMsg 	*pMsg;
    char 			*pData;
    int				extra, len;
    IceReplyWaitInfo		replyWait;
    _SmcRegisterClientReply	reply;
    Bool			gotReply;

    *clientIdRet = NULL;

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    if (!callbacks || !callbacks->save_yourself.callback ||
        !callbacks->die.callback || !callbacks->shutdown_cancelled.callback)
    {
	/* We need callbacks!  Otherwise, we can't do anything */

	strncpy (errorStringRet,
	    "All of the callbacks must be specified", errorLength);
	return (NULL);
    }

    if (!_SmcOpcode)
    {
	if ((_SmcOpcode = IceRegisterForProtocolSetup ("XSMP",
	    SmVendorString, SmReleaseString, _SmVersionCount, _SmcVersions,
            _SmAuthCount, _SmcAuthRecs, NULL)) < 0)
	{
	    strncpy (errorStringRet,
	        "Could not register XSMP protocol with ICE", errorLength);

	    return (NULL);
	}
    }

    if (networkIdsList == NULL || *networkIdsList == '\0')
    {
	if ((ids = (char *) getenv ("SESSION_MANAGER")) == NULL)
	{
	    strncpy (errorStringRet,
	        "SESSION_MANAGER environment variable not defined",
		errorLength);

	    return (NULL);
	}
    }
    else
    {
	ids = networkIdsList;
    }

    if ((iceConn = IceOpenConnection (
	ids, 0, errorLength, errorStringRet)) == NULL)
    {
	return (NULL);
    }

    setupstat = IceProtocolSetup (iceConn,
	_SmcOpcode, _SmAuthCount, NULL,	0, &majorVersion, &minorVersion,
	&vendor, &release, errorLength, errorStringRet);

    if (setupstat == IceProtocolSetupFailure)
    {
	IceCloseConnection (iceConn);
	return (NULL);
    }
    else if (setupstat == IceProtocolAlreadyActive)
    {
	/*
	 * The protocol is already active on this connection.
	 * Return the previously opened connection object.
	 */

	int i;

	for (i = 0; i < _SmcConnectionCount; i++)
	    if (_SmcConnectionObjs[i]->iceConn == iceConn)
		return (_SmcConnectionObjs[i]);

	/*
	 * If we get here, there is an error.
	 */

	IceProtocolShutdown (iceConn, _SmcOpcode);
	IceCloseConnection (iceConn);
	return (NULL);
    }

    if ((smcConn = (SmcConn) malloc (sizeof (struct _SmcConn))) == NULL)
    {
	strncpy (errorStringRet, "Can't malloc", errorLength);
	IceProtocolShutdown (iceConn, _SmcOpcode);
	IceCloseConnection (iceConn);
	free (vendor);
	free (release);
	return (NULL);
    }

    smcConn->iceConn = iceConn;
    smcConn->proto_major_version = majorVersion;
    smcConn->proto_minor_version = minorVersion;
    smcConn->vendor = vendor;
    smcConn->release = release;
    smcConn->client_id = NULL;

    memcpy (&smcConn->callbacks, callbacks, sizeof (SmcCallbacks));

    smcConn->interact_waits = NULL;
    smcConn->prop_reply_waits = NULL;

    smcConn->shutdown_in_progress = False;

    _SmcConnectionObjs[_SmcConnectionCount++] = smcConn;


    /*
     * Now register the client
     */

    len = previousId ? strlen (previousId) : 0;
    extra = ARRAY8_BYTES (len);

    IceGetHeaderExtra (iceConn, _SmcOpcode, SM_RegisterClient,
	SIZEOF (smRegisterClientMsg), WORD64COUNT (extra),
	smRegisterClientMsg, pMsg, pData);

    STORE_ARRAY8 (pData, len, previousId);

    IceFlush (iceConn);

    replyWait.sequence_of_request = IceLastSequenceNumber (iceConn);
    replyWait.major_opcode_of_request = _SmcOpcode;
    replyWait.minor_opcode_of_request = SM_RegisterClient;
    replyWait.reply = (IcePointer) &reply;

    gotReply = False;

    while (gotReply == False)
	if ((gotReply = IceProcessMessage (iceConn, &replyWait)) == True)
	{
	    if (reply.status == 1)
	    {
		/*
		 * The client successfully registered.
		 */

		*clientIdRet = reply.client_id;

		smcConn->client_id = (char *) malloc (
		    strlen (*clientIdRet) + 1);

		strcpy (smcConn->client_id, *clientIdRet);
	    }
	    else
	    {
		/*
		 * Could not register the client because the previous ID
		 * was bad.  So now we register the client with the
		 * previous ID set to NULL.
		 */

		extra = ARRAY8_BYTES (0);

		IceGetHeaderExtra (iceConn, _SmcOpcode, SM_RegisterClient,
		    SIZEOF (smRegisterClientMsg), WORD64COUNT (extra),
		    smRegisterClientMsg, pMsg, pData);

		STORE_ARRAY8 (pData, 0, NULL);

		IceFlush (iceConn);

		replyWait.sequence_of_request =
		    IceLastSequenceNumber (iceConn);

		gotReply = False;
	    }
	}

    return (smcConn);
}



void
SmcCloseConnection (smcConn, count, reasonMsgs)

SmcConn smcConn;
int	count;
char    **reasonMsgs;

{
    IceConn	iceConn = smcConn->iceConn;
    int		i;
    char	*locale;

    if (IceCheckShutdownNegotiation (iceConn) == True)
    {
	smCloseConnectionMsg 	*pMsg;
	char 			*pData;
	int			extra;

	locale = setlocale (LC_CTYPE, NULL);

	extra = 8 + ARRAY8_BYTES (strlen (locale));

	for (i = 0; i < count; i++)
	    extra += ARRAY8_BYTES (strlen (reasonMsgs[i]));

	IceGetHeaderExtra (iceConn, _SmcOpcode, SM_CloseConnection,
	    SIZEOF (smCloseConnectionMsg), WORD64COUNT (extra),
	    smCloseConnectionMsg, pMsg, pData);

	STORE_ARRAY8 (pData, strlen (locale), locale);

	STORE_CARD32 (pData, count);
	pData += 4;

	for (i = 0; i < count; i++)
	    STORE_ARRAY8 (pData, strlen (reasonMsgs[i]), reasonMsgs[i]); 

	IceFlush (iceConn);
    }

    IceProtocolShutdown (iceConn, _SmcOpcode);
    IceSetShutdownNegotiation (iceConn, False);
    IceCloseConnection (iceConn);

    for (i = 0; i < _SmcConnectionCount; i++)
	if (_SmcConnectionObjs[i] == smcConn)
	    break;

    if (i < _SmcConnectionCount)
    {
	if (i < _SmcConnectionCount - 1)
	{
	    _SmcConnectionObjs[i] =
		_SmcConnectionObjs[_SmcConnectionCount - 1];
	}

	_SmcConnectionCount--;

	if (smcConn->vendor)
	    free (smcConn->vendor);

	if (smcConn->release)
	    free (smcConn->release);

	if (smcConn->client_id)
	    free (smcConn->client_id);

	if (smcConn->prop_reply_waits)
	{
	    _SmcPropReplyWait *ptr = smcConn->prop_reply_waits;
	    _SmcPropReplyWait *next;

	    while (ptr)
	    {
		next = ptr->next;
		free ((char *) ptr);
		ptr = next;
	    }

	}

	free ((char *) smcConn);
    }
}



void
SmcModifyCallbacks (smcConn, mask, callbacks)

SmcConn    	smcConn;
unsigned long 	mask;
SmcCallbacks	*callbacks;

{
    if (mask & SmcSaveYourselfProcMask)
    {
	smcConn->callbacks.save_yourself.callback =
	    callbacks->save_yourself.callback;
	smcConn->callbacks.save_yourself.client_data =
	    callbacks->save_yourself.client_data;
    }

    if (mask & SmcDieProcMask)
    {
	smcConn->callbacks.die.callback = callbacks->die.callback;
	smcConn->callbacks.die.client_data = callbacks->die.client_data;
    }

    if (mask & SmcShutdownCancelledProcMask)
    {
	smcConn->callbacks.shutdown_cancelled.callback =
	    callbacks->shutdown_cancelled.callback;
	smcConn->callbacks.shutdown_cancelled.client_data =
	    callbacks->shutdown_cancelled.client_data;
    }
}



void
SmcSetProperties (smcConn, sequenceRef, numProps, props)

SmcConn    	smcConn;
unsigned long 	sequenceRef;
int      	numProps;
SmProp       	**props;

{
    IceConn		iceConn = smcConn->iceConn;
    smSetPropertiesMsg	*pMsg;
    char		*pBuf;
    char		*pStart;
    int			bytes;

    IceGetHeader (iceConn, _SmcOpcode, SM_SetProperties,
	SIZEOF (smSetPropertiesMsg), smSetPropertiesMsg, pMsg);

    LISTOF_PROP_BYTES (numProps, props, bytes);
    pMsg->length += WORD64COUNT (bytes);
    pMsg->sequenceRef = sequenceRef;

    pBuf = pStart = IceAllocScratch (iceConn, bytes);

    STORE_LISTOF_PROPERTY (pBuf, numProps, props);

    IceWriteData (iceConn, bytes, pStart);
    IceFlush (iceConn);
}



void
SmcGetProperties (smcConn, propReplyProc, clientData)

SmcConn		 smcConn;
SmcPropReplyProc propReplyProc;
SmPointer	 clientData;

{
    IceConn		iceConn = smcConn->iceConn;
    _SmcPropReplyWait 	*wait, *ptr;

    wait = (_SmcPropReplyWait *) malloc (sizeof (_SmcPropReplyWait));
    wait->prop_reply_proc = propReplyProc;
    wait->client_data = clientData;
    wait->next = NULL;

    ptr = smcConn->prop_reply_waits;
    while (ptr && ptr->next)
	ptr = ptr->next;

    if (ptr == NULL)
	smcConn->prop_reply_waits = wait;
    else
	ptr->next = wait;

    IceSimpleMessage (iceConn, _SmcOpcode, SM_GetProperties);
    IceFlush (iceConn);
}



void
SmcInteractRequest (smcConn, dialogType, interactProc, clientData)

SmcConn 	smcConn;
int		dialogType;
SmcInteractProc	interactProc;
SmPointer	clientData;

{
    IceConn			iceConn = smcConn->iceConn;
    smInteractRequestMsg	*pMsg;
    _SmcInteractWait 		*wait, *ptr;

    wait = (_SmcInteractWait *) malloc (sizeof (_SmcInteractWait));
    wait->interact_proc = interactProc;
    wait->client_data = clientData;
    wait->next = NULL;

    ptr = smcConn->interact_waits;
    while (ptr && ptr->next)
	ptr = ptr->next;

    if (ptr == NULL)
	smcConn->interact_waits = wait;
    else
	ptr->next = wait;

    IceGetHeader (iceConn, _SmcOpcode, SM_InteractRequest,
	SIZEOF (smInteractRequestMsg), smInteractRequestMsg, pMsg);

    pMsg->dialogType = dialogType;

    IceFlush (iceConn);
}



void
SmcInteractDone (smcConn, cancelShutdown)

SmcConn smcConn;
Bool 	cancelShutdown;

{
    IceConn		iceConn = smcConn->iceConn;
    smInteractDoneMsg	*pMsg;

    IceGetHeader (iceConn, _SmcOpcode, SM_InteractDone,
	SIZEOF (smInteractDoneMsg), smInteractDoneMsg, pMsg);

    pMsg->cancelShutdown = cancelShutdown;

    IceFlush (iceConn);
}



void
SmcSaveYourselfDone (smcConn, success)

SmcConn smcConn;
Bool	success;

{
    IceConn			iceConn = smcConn->iceConn;
    smSaveYourselfDoneMsg	*pMsg;

    IceGetHeader (iceConn, _SmcOpcode, SM_SaveYourselfDone,
	SIZEOF (smSaveYourselfDoneMsg), smSaveYourselfDoneMsg, pMsg);

    pMsg->success = success;

    IceFlush (iceConn);
}
