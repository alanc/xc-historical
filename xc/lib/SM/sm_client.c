/* $XConsortium: client.c,v 1.1 93/09/03 13:25:08 mor Exp $ */
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
            _SmAuthCount, _SmcAuthRecs)) < 0)
	{
	   return (0);
	}
    }

    bcopy ((char *) callbacks,
	(char *) &_SmcCallbacks, sizeof (SmcCallbacks));

    return (1);
}



SmcConn
SmcOpenConnection (networkIdsList, callData, previousIdLen, previousId,
    clientIdLenRet, clientIdRet, errorLength, errorStringRet)

char 		*networkIdsList;
SmPointer	callData;
int  		previousIdLen;
char 		*previousId;
int  		*clientIdLenRet;
char 		**clientIdRet;
int  		errorLength;
char 		*errorStringRet;

{
    SmcConn		smcConn;
    IceConn		iceConn;
    char 		*ids;
    smRegisterClientMsg *pMsg;
    char 		*pData;
    int			extra;
    IceReplyWaitInfo	replyWait;
    SmcReply		reply;
    Bool		gotReply;

    *clientIdLenRet = 0;
    *clientIdRet = NULL;

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

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

    if ((smcConn = (SmcConn) malloc (sizeof (struct _SmcConn))) == NULL)
    {
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }

    if ((smcConn->iceConn = iceConn = IceOpenConnection (
	ids, errorLength, errorStringRet)) == NULL)
    {
	free ((char *) smcConn);
	return (NULL);
    }

    smcConn->call_data = callData;
    smcConn->interact_cb = NULL;
    smcConn->prop_reply_waits = NULL;

    if (!IceProtocolSetup (iceConn, _SmcOpcode,	_SmAuthCount, NULL,
	&smcConn->proto_major_version, &smcConn->proto_minor_version,
	&smcConn->vendor, &smcConn->release, errorLength, errorStringRet))
    {
	IceCloseConnection (iceConn);
	free ((char *) smcConn);
	return (NULL);
    }

    _SmcConnectionObjs[_SmcConnectionCount++] = smcConn;


    /*
     * Now register the client
     */

    extra = ARRAY8_BYTES (previousIdLen);

    IceGetHeaderExtra (iceConn, _SmcOpcode, SM_RegisterClient,
	SIZEOF (smRegisterClientMsg), WORD64COUNT (extra),
	smRegisterClientMsg, pMsg, pData);

    STORE_ARRAY8 (pData, previousIdLen, previousId);
    IceFlush (iceConn);

    replyWait.sequence_of_request = IceLastSequenceNumber (iceConn);
    replyWait.major_opcode_of_request = _SmcOpcode;
    replyWait.minor_opcode_of_request = SM_RegisterClient;
    replyWait.reply = (char *) &reply;

    gotReply = False;

    while (gotReply == False)
	if ((gotReply = IceProcessMessage (iceConn, &replyWait)) == True)
	{
	    if (reply.type == SMC_REGISTER_CLIENT_REPLY)
	    {
		*clientIdLenRet = reply.register_client_reply.client_id_len;
		*clientIdRet = reply.register_client_reply.client_id;

		smcConn->client_id_len = *clientIdLenRet;
		smcConn->client_id = (char *) malloc (*clientIdLenRet);
		bcopy (*clientIdRet, smcConn->client_id, *clientIdLenRet);
	    }
	    else /* reply.type == SMC_ERROR_REPLY */
	    {
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
    IceConn			iceConn = smcConn->iceConn;
    smCloseConnectionMsg 	*pMsg;
    char 			*pData;
    int				extra, i;

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

	if (smcConn->client_id)
	    free (smcConn->client_id);

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
    if (smcConn->interact_cb == NULL)
    {
	IceConn			iceConn = smcConn->iceConn;
	smInteractRequestMsg	*pMsg;

	IceGetHeader (iceConn, _SmcOpcode, SM_InteractRequest,
	    SIZEOF (smInteractRequestMsg), smInteractRequestMsg, pMsg);

	pMsg->dialogType = dialogType;

	IceFlush (iceConn);

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