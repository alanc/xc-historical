/* $XConsortium: sm_process.c,v 1.3 93/09/08 20:23:34 mor Exp $ */
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
	return (0);

    if (!smcConn->client_id &&
        opcode != SM_RegisterClientReply && opcode != SM_Error)
    {
	_SmcErrorBadState (iceConn, opcode, IceFatalToProtocol);
	return (0);
    }

    switch (opcode)
    {
    case SM_Error:
    {
	iceErrorMsg 	*pMsg;
	char	    	*pData;

	IceReadCompleteMessage (iceConn,
	    SIZEOF (iceErrorMsg), iceErrorMsg, pMsg, pData);

	if (replyWait &&
	    replyWait->minor_opcode_of_request == SM_RegisterClient &&
	    pMsg->offendingMinorOpcode == SM_RegisterClient &&
	    pMsg->offendingSequenceNum == replyWait->sequence_of_request)
	{
	    _SmcRegisterClientReply *reply =
		(_SmcRegisterClientReply *) (replyWait->reply);

	    reply->status = 0;

	    replyReady = True;
	}
	else
	{
	    (*_SmcErrorHandler) (smcConn,
		pMsg->offendingMinorOpcode,
	        pMsg->offendingSequenceNum,
		pMsg->errorClass, pMsg->severity,
		(SmPointer) pData);
	}
	break;
    }

    case SM_RegisterClientReply:

	if (!replyWait ||
	    replyWait->minor_opcode_of_request != SM_RegisterClient)
	{
	    _SmcErrorBadState (iceConn,
		SM_RegisterClientReply, IceFatalToProtocol);
	}
        else
	{
	    smRegisterClientReplyMsg 	*pMsg;
	    char			*pData;
	    _SmcRegisterClientReply 	*reply = 
	        (_SmcRegisterClientReply *) (replyWait->reply);

	    IceReadCompleteMessage (iceConn,
		SIZEOF (smRegisterClientReplyMsg),
		smRegisterClientReplyMsg, pMsg, pData);

	    EXTRACT_ARRAY8_AS_STRING (pData, reply->client_id);

	    reply->status = 1;
	    replyReady = True;
	}
	break;

    case SM_SaveYourself:
    {
	smSaveYourselfMsg 	*pMsg;

	IceReadMessageHeader (iceConn, SIZEOF (smSaveYourselfMsg),
	    smSaveYourselfMsg, pMsg);

	(*_SmcCallbacks.save_yourself) (smcConn, smcConn->client_data,
	    pMsg->saveType, pMsg->shutdown, pMsg->interactStyle, pMsg->fast);
	break;
    }

    case SM_Interact:

        if (!smcConn->interact_cb)
	{
	    _SmcErrorBadState (iceConn,	SM_Interact, IceCanContinue);
	}
        else
	{
	    (*smcConn->interact_cb) (smcConn, smcConn->client_data);
	    smcConn->interact_cb = NULL;
	}
	break;

    case SM_Die:

	IceSkipShutdownNegotiation (smcConn->iceConn);
	(*_SmcCallbacks.die) (smcConn, smcConn->client_data);
	break;

    case SM_ShutdownCancelled:

	(*_SmcCallbacks.shutdown_cancelled) (smcConn, smcConn->client_data);
	break;

    case SM_PropertiesReply:

        if (!smcConn->prop_reply_waits)
	{
	    _SmcErrorBadState (iceConn,
		SM_PropertiesReply, IceCanContinue);
	}
        else
	{
	    smPropertiesReplyMsg 	*pMsg;
	    char 			*pData;
	    int				numProps;
	    SmProp			*props;
	    _SmcPropReplyWait 		*next;

	    IceReadCompleteMessage (iceConn,
		SIZEOF (smPropertiesReplyMsg),
		smPropertiesReplyMsg, pMsg, pData);

	    EXTRACT_LISTOF_PROPERTY (pData, numProps, props);

	    next = smcConn->prop_reply_waits->next;

	    (*smcConn->prop_reply_waits->prop_reply_cb) (smcConn,
		smcConn->client_data, numProps, props);

	    free ((char *) smcConn->prop_reply_waits);
	    smcConn->prop_reply_waits = next;
	}
	break;

    default:
    {
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
	return;

    if (!smsConn->client_id &&
        opcode != SM_RegisterClient && opcode != SM_Error)
    {
	_SmsErrorBadState (iceConn, opcode, IceFatalToProtocol);
	return;
    }

    switch (opcode)
    {
    case SM_Error:
    {
	iceErrorMsg 	*pMsg;
	char	    	*pData;

	IceReadCompleteMessage (iceConn,
	    SIZEOF (iceErrorMsg), iceErrorMsg, pMsg, pData);

	(*_SmsErrorHandler) (smsConn,
	    pMsg->offendingMinorOpcode,
	    pMsg->offendingSequenceNum,
	    pMsg->errorClass, pMsg->severity,
            (SmPointer) pData);
	break;
    }

    case SM_RegisterClient:
    {
	smRegisterClientMsg 	*pMsg;
	char 			*pData;
	char 			*previousId = NULL;

	IceReadCompleteMessage (iceConn,
	    SIZEOF (smRegisterClientMsg),
	    smRegisterClientMsg, pMsg, pData);

	EXTRACT_ARRAY8_AS_STRING (pData, previousId);

	(*_SmsCallbacks.register_client) (smsConn,
            smsConn->manager_data, previousId);

	break;
    }

    case SM_InteractRequest:

        if (!smsConn->save_yourself_in_progress ||
	    smsConn->waiting_to_interact)
	{
	    _SmsErrorBadState (iceConn,
		SM_InteractRequest, IceCanContinue);
	}
        else
	{
	    smInteractRequestMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smInteractRequestMsg, pMsg);

	    smsConn->waiting_to_interact = True;

	    (*_SmsCallbacks.interact_request) (smsConn,
	        smsConn->manager_data, pMsg->dialogType);
	}
	break;

    case SM_InteractDone:

        if (!smsConn->interact_in_progress)
	{
	    _SmsErrorBadState (iceConn,
		SM_InteractDone, IceCanContinue);
	}
        else
	{
	    smInteractDoneMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smInteractDoneMsg, pMsg);

	    smsConn->interact_in_progress = False;

	    (*_SmsCallbacks.interact_done) (smsConn,
	      smsConn->manager_data, pMsg->cancelShutdown);
	}
	break;

    case SM_SaveYourselfDone:

        if (!smsConn->save_yourself_in_progress)
	{
	    _SmsErrorBadState (iceConn,
		SM_SaveYourselfDone, IceCanContinue);
	}
        else
	{
	    smSaveYourselfDoneMsg 	*pMsg;

	    IceReadSimpleMessage (iceConn, smSaveYourselfDoneMsg, pMsg);

	    smsConn->save_yourself_in_progress = False;

	    (*_SmsCallbacks.save_yourself_done) (smsConn,
	        smsConn->manager_data, pMsg->success);
	}
	break;

    case SM_CloseConnection:
    {
	smCloseConnectionMsg 	*pMsg;
	char 			*pData;
	char 			*locale = NULL;
	int 			count, i;
	char 			**reasonMsgs = NULL;

	IceReadCompleteMessage (iceConn,
	    SIZEOF (smCloseConnectionMsg),
	    smCloseConnectionMsg, pMsg, pData);

	EXTRACT_ARRAY8_AS_STRING (pData, locale);

	EXTRACT_CARD32 (pData, count);
	pData += 4;

	reasonMsgs = (char **) malloc (count * sizeof (char *));
	for (i = 0; i < count; i++)
	    EXTRACT_ARRAY8_AS_STRING (pData, reasonMsgs[i]);

	(*_SmsCallbacks.close_connection) (smsConn,
	    smsConn->manager_data, locale, count, reasonMsgs);

	break;
    }

    case SM_SetProperties:
    {
	smSetPropertiesMsg 	*pMsg;
	char 			*pData;
	SmProp			*props = NULL;
	int 			numProps;
	
	IceReadCompleteMessage (iceConn,
	    SIZEOF (smSetPropertiesMsg),
	    smSetPropertiesMsg, pMsg, pData);

	EXTRACT_LISTOF_PROPERTY (pData, numProps, props);

	(*_SmsCallbacks.set_properties) (smsConn,
	    smsConn->manager_data, pMsg->sequenceRef, numProps, props);

	break;
    }

    case SM_GetProperties:

	(*_SmsCallbacks.get_properties) (smsConn, smsConn->manager_data);
	break;

    default:
    {
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
