/* $XConsortium: sm_process.c,v 1.12 93/09/26 18:15:00 mor Exp $ */
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


Bool
_SmcProcessMessage (iceConn, opcode, length, swap, replyWait)

IceConn		 iceConn;
int		 opcode;
unsigned long	 length;
Bool		 swap;
IceReplyWaitInfo *replyWait;

{
    SmcConn	smcConn = NULL;
    Bool	replyReady = False;
    int		i;

    for (i = 0; i < _SmcConnectionCount; i++)
	if (_SmcConnectionObjs[i]->iceConn == iceConn)
	{
	    smcConn = _SmcConnectionObjs[i];
	    break;
	}

    if (smcConn == NULL)
    {
	_IceReadSkip (iceConn, length << 3);

	return (0);
    }

    if (!smcConn->client_id &&
        opcode != SM_RegisterClientReply && opcode != SM_Error)
    {
	_IceReadSkip (iceConn, length << 3);

	_IceErrorBadState (iceConn, _SmcOpcode, opcode, IceFatalToProtocol);

	return (0);
    }

    switch (opcode)
    {
    case SM_Error:
    {
	iceErrorMsg 	*pMsg;
	char	    	*pData;

	IceReadCompleteMessage (iceConn, SIZEOF (iceErrorMsg),
	    iceErrorMsg, pMsg, pData);

	if (replyWait &&
	    replyWait->minor_opcode_of_request == SM_RegisterClient &&
            pMsg->errorClass == IceBadValue &&
	    pMsg->offendingMinorOpcode == SM_RegisterClient &&
	    pMsg->offendingSequenceNum == replyWait->sequence_of_request)
	{
	    /*
	     * For Register Client, the previous ID was bad.
	     */

	    _SmcRegisterClientReply *reply =
		(_SmcRegisterClientReply *) (replyWait->reply);

	    reply->status = 0;

	    replyReady = True;
	}
	else
	{
	    (*_SmcErrorHandler) (smcConn, swap,
		pMsg->offendingMinorOpcode,
	        pMsg->offendingSequenceNum,
		pMsg->errorClass, pMsg->severity,
		(SmPointer) pData);
	}

	IceDisposeCompleteMessage (iceConn, pData);
	break;
    }

    case SM_RegisterClientReply:

	if (!replyWait ||
	    replyWait->minor_opcode_of_request != SM_RegisterClient)
	{
	    _IceReadSkip (iceConn, length << 3);

	    _IceErrorBadState (iceConn, _SmcOpcode,
		SM_RegisterClientReply, IceFatalToProtocol);
	}
        else
	{
	    smRegisterClientReplyMsg 	*pMsg;
	    char			*pData;
	    _SmcRegisterClientReply 	*reply = 
	        (_SmcRegisterClientReply *) (replyWait->reply);

	    IceReadCompleteMessage (iceConn, SIZEOF (smRegisterClientReplyMsg),
		smRegisterClientReplyMsg, pMsg, pData);

	    EXTRACT_ARRAY8_AS_STRING (pData, swap, reply->client_id);

	    reply->status = 1;
	    replyReady = True;

	    IceDisposeCompleteMessage (iceConn, pData);
	}
	break;

    case SM_SaveYourself:
    {
	smSaveYourselfMsg 	*pMsg;

	IceReadMessageHeader (iceConn, SIZEOF (smSaveYourselfMsg),
	    smSaveYourselfMsg, pMsg);

	(*smcConn->callbacks.save_yourself.callback) (smcConn,
	    smcConn->callbacks.save_yourself.client_data,
            pMsg->saveType, pMsg->shutdown, pMsg->interactStyle, pMsg->fast);

	if (pMsg->shutdown)
	    smcConn->shutdown_in_progress = True;
	break;
    }

    case SM_Interact:

        if (!smcConn->interact_waits)
	{
	    _IceErrorBadState (iceConn, _SmcOpcode,
		SM_Interact, IceCanContinue);
	}
        else
	{
	    _SmcInteractWait *next = smcConn->interact_waits->next;

	    (*smcConn->interact_waits->interact_proc) (smcConn,
		smcConn->interact_waits->client_data);

	    free ((char *) smcConn->interact_waits);
	    smcConn->interact_waits = next;
	}
	break;

    case SM_Die:

	IceSetShutdownNegotiation (smcConn->iceConn, False);

	(*smcConn->callbacks.die.callback) (smcConn,
	    smcConn->callbacks.die.client_data);
	break;

    case SM_ShutdownCancelled:

	if (!smcConn->shutdown_in_progress)
	{
	    _IceErrorBadState (iceConn, _SmcOpcode,
		SM_ShutdownCancelled, IceCanContinue);
	}
	else
	{
	    smcConn->shutdown_in_progress = False;

	    (*smcConn->callbacks.shutdown_cancelled.callback) (smcConn,
	        smcConn->callbacks.shutdown_cancelled.client_data);
	}
	break;

    case SM_PropertiesReply:

        if (!smcConn->prop_reply_waits)
	{
	    _IceReadSkip (iceConn, length << 3);

	    _IceErrorBadState (iceConn, _SmcOpcode,
		SM_PropertiesReply, IceCanContinue);
	}
        else
	{
	    smPropertiesReplyMsg 	*pMsg;
	    char 			*pData;
	    int				numProps;
	    SmProp			*props;
	    _SmcPropReplyWait 		*next;

	    IceReadCompleteMessage (iceConn, SIZEOF (smPropertiesReplyMsg),
		smPropertiesReplyMsg, pMsg, pData);

	    EXTRACT_LISTOF_PROPERTY (pData, swap, numProps, props);

	    next = smcConn->prop_reply_waits->next;

	    (*smcConn->prop_reply_waits->prop_reply_proc) (smcConn,
		smcConn->prop_reply_waits->client_data, numProps, props);

	    free ((char *) smcConn->prop_reply_waits);
	    smcConn->prop_reply_waits = next;

	    IceDisposeCompleteMessage (iceConn, pData);
	}
	break;

    default:
    {
	_IceReadSkip (iceConn, length << 3);

	IceErrorHeader (iceConn,
	    _SmcOpcode, opcode,
	    iceConn->sequence - 1,
	    IceCanContinue,
	    IceBadMinor,
	    0);

	IceFlush (iceConn);
	break;
    }
    }

    return (replyReady);
}


void
_SmsProcessMessage (iceConn, opcode, length, swap)

IceConn		 iceConn;
int		 opcode;
unsigned long	 length;
Bool		 swap;

{
    SmsConn	smsConn = NULL;
    int		i;

    for (i = 0; i < _SmsConnectionCount; i++)
	if (_SmsConnectionObjs[i]->iceConn == iceConn)
	{
	    smsConn = _SmsConnectionObjs[i];
	    break;
	}

    if (smsConn == NULL)
    {
	_IceReadSkip (iceConn, length << 3);

	return;
    }

    if (!smsConn->client_id &&
        opcode != SM_RegisterClient && opcode != SM_Error)
    {
	_IceReadSkip (iceConn, length << 3);

	_IceErrorBadState (iceConn, _SmsOpcode, opcode, IceFatalToProtocol);

	return;
    }

    switch (opcode)
    {
    case SM_Error:
    {
	iceErrorMsg 	*pMsg;
	char	    	*pData;

	IceReadCompleteMessage (iceConn, SIZEOF (iceErrorMsg),
	    iceErrorMsg, pMsg, pData);

	(*_SmsErrorHandler) (smsConn, swap,
	    pMsg->offendingMinorOpcode,
	    pMsg->offendingSequenceNum,
	    pMsg->errorClass, pMsg->severity,
            (SmPointer) pData);

	IceDisposeCompleteMessage (iceConn, pData);
	break;
    }

    case SM_RegisterClient:
    {
	smRegisterClientMsg 	*pMsg;
	char 			*pData;
	char 			*savePtr;
	char 			*previousId;

	IceReadCompleteMessage (iceConn, SIZEOF (smRegisterClientMsg),
	    smRegisterClientMsg, pMsg, pData);

	savePtr = pData;

	EXTRACT_ARRAY8_AS_STRING (pData, swap, previousId);

	if (*previousId == '\0')
	{
	    free (previousId);
	    previousId = NULL;
	}

	if (!(*smsConn->callbacks.register_client.callback) (smsConn,
            smsConn->callbacks.register_client.manager_data, previousId))
	{
	    /*
	     * The previoudId was bad.  Generate BadValue error.
	     */

	    int length = previousId ? strlen (previousId) : 0;
	    int bytes = ARRAY8_BYTES (length);

	    _IceErrorBadValue (smsConn->iceConn, _SmsOpcode, SM_RegisterClient,
		8, bytes, (IcePointer) savePtr);
	}

	IceDisposeCompleteMessage (iceConn, pData);
	break;
    }

    case SM_InteractRequest:

        if (!smsConn->save_yourself_in_progress ||
	    smsConn->interaction_allowed == SmInteractStyleNone)
	{
	    _IceErrorBadState (iceConn, _SmsOpcode,
		SM_InteractRequest, IceCanContinue);
	}
        else
	{
	    smInteractRequestMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smInteractRequestMsg, pMsg);

	    if (pMsg->dialogType == SmDialogNormal &&
		smsConn->interaction_allowed != SmInteractStyleAny)
	    {
		_IceErrorBadState (iceConn, _SmsOpcode,
		    SM_InteractRequest, IceCanContinue);
	    }
	    else
	    {
		(*smsConn->callbacks.interact_request.callback) (smsConn,
	            smsConn->callbacks.interact_request.manager_data,
		    pMsg->dialogType);
	    }
	}
	break;

    case SM_InteractDone:

        if (!smsConn->interact_in_progress)
	{
	    _IceErrorBadState (iceConn, _SmsOpcode,
		SM_InteractDone, IceCanContinue);
	}
        else
	{
	    smInteractDoneMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smInteractDoneMsg, pMsg);

	    if (pMsg->cancelShutdown && !smsConn->can_cancel_shutdown)
	    {
		_IceErrorBadState (iceConn, _SmsOpcode,
		    SM_InteractDone, IceCanContinue);
	    }
	    else
	    {
		smsConn->interact_in_progress = False;

		(*smsConn->callbacks.interact_done.callback) (smsConn,
	            smsConn->callbacks.interact_done.manager_data,
	            pMsg->cancelShutdown);
	    }
	}
	break;

    case SM_SaveYourselfDone:

        if (!smsConn->save_yourself_in_progress)
	{
	    _IceErrorBadState (iceConn, _SmsOpcode,
		SM_SaveYourselfDone, IceCanContinue);
	}
        else
	{
	    smSaveYourselfDoneMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smSaveYourselfDoneMsg, pMsg);

	    smsConn->save_yourself_in_progress = False;
	    smsConn->interaction_allowed = SmInteractStyleNone;

	    (*smsConn->callbacks.save_yourself_done.callback) (smsConn,
	        smsConn->callbacks.save_yourself_done.manager_data,
		pMsg->success);
	}
	break;

    case SM_CloseConnection:
    {
	smCloseConnectionMsg 	*pMsg;
	char 			*pData;
	char 			*locale = NULL;
	int 			count, i;
	char 			**reasonMsgs = NULL;

	IceReadCompleteMessage (iceConn, SIZEOF (smCloseConnectionMsg),
	    smCloseConnectionMsg, pMsg, pData);

	EXTRACT_ARRAY8_AS_STRING (pData, swap, locale);

	EXTRACT_CARD32 (pData, swap, count);
	pData += 4;

	reasonMsgs = (char **) malloc (count * sizeof (char *));
	for (i = 0; i < count; i++)
	    EXTRACT_ARRAY8_AS_STRING (pData, swap, reasonMsgs[i]);

	(*smsConn->callbacks.close_connection.callback) (smsConn,
	    smsConn->callbacks.close_connection.manager_data,
	     locale, count, reasonMsgs);

	IceDisposeCompleteMessage (iceConn, pData);
	break;
    }

    case SM_SetProperties:
    {
	smSetPropertiesMsg 	*pMsg;
	char 			*pData;
	SmProp			*props = NULL;
	int 			numProps;
	
	IceReadCompleteMessage (iceConn, SIZEOF (smSetPropertiesMsg),
	    smSetPropertiesMsg, pMsg, pData);

	if (swap)
	    pMsg->sequenceRef = lswapl (pMsg->sequenceRef);

	EXTRACT_LISTOF_PROPERTY (pData, swap, numProps, props);

	(*smsConn->callbacks.set_properties.callback) (smsConn,
	    smsConn->callbacks.set_properties.manager_data,
            pMsg->sequenceRef, numProps, props);

	IceDisposeCompleteMessage (iceConn, pData);
	break;
    }

    case SM_GetProperties:

	(*smsConn->callbacks.get_properties.callback) (smsConn,
	    smsConn->callbacks.get_properties.manager_data);
	break;

    default:
    {
	_IceReadSkip (iceConn, length << 3);

	IceErrorHeader (iceConn,
	    _SmsOpcode, opcode,
	    iceConn->sequence - 1,
	    IceCanContinue,
	    IceBadMinor,
	    0);

	IceFlush (iceConn);
	break;
    }
    }
}
