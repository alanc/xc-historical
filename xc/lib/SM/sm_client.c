/* $XConsortium: sm_client.c,v 1.5 93/09/12 16:23:06 mor Exp $ */
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

Status
SmcInitialize (callbacks)

SmcCallbacks	*callbacks;

{
    if (!callbacks || !callbacks->save_yourself ||
        !callbacks->die || !callbacks->shutdown_cancelled)
    {
	/* We need callbacks!  Otherwise, we can't do anything */

	return (0);
    }

    if (!_SmcOpcode)
    {
	if ((_SmcOpcode = IceRegisterForProtocolSetup ("XSMP",
	    SmVendorString, SmReleaseString, _SmVersionCount, _SmcVersions,
            _SmAuthCount, _SmcAuthRecs, NULL)) < 0)
	{
	   return (0);
	}
    }

    bcopy ((char *) callbacks,
	(char *) &_SmcCallbacks, sizeof (SmcCallbacks));

    return (1);
}



SmcConn
SmcOpenConnection (networkIdsList, clientData,
    previousId, clientIdRet, errorLength, errorStringRet)

char 		*networkIdsList;
SmPointer	clientData;
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
    int				extra;
    IceReplyWaitInfo		replyWait;
    _SmcRegisterClientReply	reply;
    Bool			gotReply;

    *clientIdRet = NULL;

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    if (!_SmcOpcode)
    {
	strncpy (errorStringRet, "SmcInitialize was not called", errorLength);
	return (NULL);
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
	ids, errorLength, errorStringRet)) == NULL)
    {
	return (NULL);
    }

    setupstat = IceProtocolSetup (iceConn,
	_SmcOpcode, _SmAuthCount, NULL,	&majorVersion, &minorVersion,
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
    smcConn->client_data = clientData;
    smcConn->client_id = NULL;
    smcConn->interact_cb = NULL;
    smcConn->prop_reply_waits = NULL;

    _SmcConnectionObjs[_SmcConnectionCount++] = smcConn;


    /*
     * Now register the client
     */

    extra = ARRAY8_BYTES (strlen (previousId));

    IceGetHeaderExtra (iceConn, _SmcOpcode, SM_RegisterClient,
	SIZEOF (smRegisterClientMsg), WORD64COUNT (extra),
	smRegisterClientMsg, pMsg, pData);

    STORE_ARRAY8 (pData, strlen (previousId), previousId);
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
		*clientIdRet = reply.client_id;

		smcConn->client_id = (char *) malloc (
		    strlen (*clientIdRet) + 1);

		strcpy (smcConn->client_id, *clientIdRet);
	    }
	    else
	    {
		IceProtocolShutdown (iceConn, _SmcOpcode);
		IceCloseConnection (iceConn);

		free (smcConn->vendor);
		free (smcConn->release);
		strncpy (errorStringRet,
		    "Failed to register client", errorLength);
		free ((char *) smcConn);

		return (NULL);
	    }
	}

    return (smcConn);
}



void
SmcCloseConnection (smcConn, locale, count, reasonMsgs)

SmcConn smcConn;
char	*locale;
int	count;
char    **reasonMsgs;

{
    IceConn	iceConn = smcConn->iceConn;
    int		i;

    if (IceCheckShutdownNegotiation (iceConn) == True)
    {
	smCloseConnectionMsg 	*pMsg;
	char 			*pData;
	int			extra;

	if (locale == NULL || *locale == '\0')
	    locale = setlocale (LC_ALL, NULL);

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
SmcSetProperties (smcConn, sequenceRef, numProps, props)

SmcConn    	smcConn;
unsigned long 	sequenceRef;
int      	numProps;
SmProp       	*props;

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
SmcGetProperties (smcConn, propReplyCB)

SmcConn		smcConn;
SmcPropReplyCB	propReplyCB;

{
    IceConn		iceConn = smcConn->iceConn;
    _SmcPropReplyWait 	*wait, *ptr;

    wait = (_SmcPropReplyWait *) malloc (sizeof (_SmcPropReplyWait));
    wait->prop_reply_cb = propReplyCB;
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
SmcInteractRequest (smcConn, dialogType, interactCB)

SmcConn 	smcConn;
int		dialogType;
SmcInteractCB	interactCB;

{
    IceConn			iceConn = smcConn->iceConn;
    smInteractRequestMsg	*pMsg;

    IceGetHeader (iceConn, _SmcOpcode, SM_InteractRequest,
	SIZEOF (smInteractRequestMsg), smInteractRequestMsg, pMsg);

    pMsg->dialogType = dialogType;

    IceFlush (iceConn);

    if (!smcConn->interact_cb)
    {
	/*
	 * There can only be one InteractRequest active for the client.
	 * If InteractRequest was already called and the Interact message
	 * has not arrived, then the client shouldn't have called this
	 * function.  The session manager should send an error message.
	 */

	smcConn->interact_cb = interactCB;
    }
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
