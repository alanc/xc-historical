/* $XConsortium: process.c,v 1.4 93/09/05 21:09:11 mor Exp $ */
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


/*
 * If replyWait == NULL, the client is not waiting for a reply.
 *
 * If replyWait != NULL, the client is waiting for a reply...
 *
 *    - replyWait->sequence_of_request is the sequence number of the
 *      message for which the client is waiting a reply.  This is needed
 *	to determine if an error matches a replyWait.
 *
 *    - replyWait->major_opcode_of_request is the major opcode of the
 *      message for which we are waiting a reply.
 *
 *    - replyWait->minor_opcode_of_request is the minor opcode of the
 *      message for which we are waiting a reply.
 *
 *    - replyWait->reply is a pointer to the reply message which will be
 *	filled in when the reply is ready (the protocol library should
 *      cast this IcePointer to the appropriate reply type).  In most cases,
 *      the reply will have some fixed-size part, and the sender function
 *      will have provided a pointer to a structure (e.g.) to hold this
 *      fixed-size data.  If there is variable-length data, it would be
 *      expected that the reply function will have to allocate additional
 *      memory and store pointer(s) to that memory in the fixed-size
 *      structure.  If the entire data is variable length (e.g., a single
 *      variable-length string), then the sender function would probably
 *      just pass a pointer to fixed-size space to hold a pointer, and the
 *      reply function would allocate the storage and store the pointer.
 *	It is the responsibility of the client receiving the reply to
 *	free up any memory allocated on it's behalf.
 *
 * We might be waiting for several different replies (a function can wait
 * for a reply, and while calling IceProcessMessage, a callback can be
 * invoked which will wait for another reply).  We take advantage of the
 * fact that for a given protocol, we are guaranteed that messages are
 * processed in the order we sent them.  So, everytime we have a new
 * replyWait, we add it to the END of the 'saved_reply_waits' list.  When
 * we read a message and want to see if it matches a replyWait, we use the
 * FIRST replyWait in the list with the major opcode of the message.  If the
 * reply is ready, we remove that replyWait from the list.
 *
 * If the reply/error is ready for the replyWait passed in to
 * IceProcessMessage, True is returned.  Otherwise, False is returned.
 */

Bool
IceProcessMessage (iceConn, replyWait)

IceConn		 iceConn;
IceReplyWaitInfo *replyWait;

{
    iceMsg		*header;
    Bool		replyReady = False;
    IceReplyWaitInfo	*useThisReplyWait = NULL;
    
    _IceRead (iceConn, SIZEOF (iceMsg), iceConn->inbuf);
    header = (iceMsg *) iceConn->inbuf;
    iceConn->inbufptr = iceConn->inbuf + SIZEOF (iceMsg);

    if (iceConn->waiting_for_byteorder)
    {
	if (header->majorOpcode == 0 &&
	    header->minorOpcode == ICE_ByteOrder)
	{
	    int byteOrder = ((iceByteOrderMsg *) header)->byteOrder;
	    int endian = 1;

	    iceConn->swap =
	        (((*(char *) &endian) && byteOrder == IceBigEndian) ||
	         !(*(char *) &endian) && byteOrder == IceLittleEndian);

	    iceConn->waiting_for_byteorder = 0;
	}
	else
	{
	    if (header->majorOpcode != 0)
	    {
		_IceErrorBadMajor (iceConn,
		    header->majorOpcode, header->minorOpcode);
	    }
	    else
	    {
		_IceErrorBadState (iceConn,
		    header->minorOpcode, IceFatalToProtocol);
	    }

	    iceConn->connection_status = IceConnectRejected;
	}

	return (False);
    }

    if (iceConn->swap)
    {
	/* swap the length field */
	;
    }

    iceConn->sequence++;

    if (replyWait)
    {
	/*
	 * Add to the list of replyWaits (only if it doesn't exist
	 * in the list already.
	 */

	_IceAddReplyWait (iceConn, replyWait);


	/*
	 * Note that there are two different replyWaits.  The first is
	 * the one passed into IceProcessMessage, and is the replyWait
	 * for the message the client is blocking on.  The second is
	 * the replyWait for the message currently being processed
	 * by IceProcessMessage.  We call it "useThisReplyWait".
	 */

	useThisReplyWait = _IceSearchReplyWaits (iceConn, header->majorOpcode);
    }

    if (header->majorOpcode == 0)
    {
	/*
	 * ICE protocol
	 */

	_IceProcessCoreMsgCB processIce =
	    _IceVersions[iceConn->my_ice_version_index].process_core_msg_cb;

	replyReady = (*processIce) (iceConn, header->minorOpcode,
	    iceConn->swap, useThisReplyWait);
    }
    else
    {
	/*
	 * Sub protocol
	 */

	if ((int) header->majorOpcode < iceConn->his_min_opcode ||
	    (int) header->majorOpcode > iceConn->his_max_opcode ||
	    !(iceConn->process_msg_info[header->majorOpcode -
	    iceConn->his_min_opcode].in_use))
	{
	    /*
	     * The protocol of the message we just read is not supported.
	     */

	    _IceErrorBadMajor (iceConn,
		header->majorOpcode, header->minorOpcode);
	}
	else
	{
	    _IceProcessMsgInfo *processMsgInfo = &iceConn->process_msg_info[
		header->majorOpcode - iceConn->his_min_opcode];

	    if (processMsgInfo->accept_flag)
	    {
		IceACLprocessMsgCB processCB =
		    processMsgInfo->process_msg_cb.accept_client;

		(*processCB) (iceConn, header->minorOpcode,
		    header->length, iceConn->swap);
	    }
	    else
	    {
		IceOCLprocessMsgCB processCB =
		    processMsgInfo->process_msg_cb.orig_client;

		replyReady = (*processCB) (iceConn, header->minorOpcode,
		    header->length, iceConn->swap, useThisReplyWait);
	    }
	}
    }

    if (replyReady)
    {
	_IceSetReplyReady (iceConn, useThisReplyWait);
    }


    /*
     * Now we check if the reply is ready for the replyWait passed
     * into IceProcessMessage.  The replyWait is removed from the
     * replyWait list if it is ready.
     */

    return (_IceCheckReplyReady (iceConn, replyWait));
}



static void
AuthRequired (iceConn, authIndex, authDataLen, authData)

IceConn		iceConn;
int  		authIndex;
int  		authDataLen;
IcePointer	authData;

{
    iceAuthRequiredMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthRequired,
	SIZEOF (iceAuthRequiredMsg), iceAuthRequiredMsg, pMsg);
    pMsg->authIndex = authIndex;
    pMsg->length += WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AuthReply (iceConn, authDataLen, authData)

IceConn		iceConn;
int 		authDataLen;
IcePointer	authData;

{
    iceAuthReplyMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthReply,
	SIZEOF (iceAuthReplyMsg), iceAuthReplyMsg, pMsg);
    pMsg->length +=  WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AuthNextPhase (iceConn, authDataLen, authData)

IceConn		iceConn;
int  		authDataLen;
IcePointer	authData;

{
    iceAuthNextPhaseMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthNextPhase,
	SIZEOF (iceAuthNextPhaseMsg), iceAuthNextPhaseMsg, pMsg);
    pMsg->length += WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AcceptConnection (iceConn, versionIndex)

IceConn iceConn;
int 	versionIndex;

{
    iceConnectionReplyMsg	*pMsg;
    char			*pData;
    int				extra;

    extra = XPCS_BYTES (IceVendorString) + XPCS_BYTES (IceReleaseString);

    IceGetHeaderExtra (iceConn, 0, ICE_ConnectionReply,
	SIZEOF (iceConnectionReplyMsg), WORD64COUNT (extra),
	iceConnectionReplyMsg, pMsg, pData);

    pMsg->versionIndex = versionIndex;

    STORE_XPCS (pData, IceVendorString);
    STORE_XPCS (pData, IceReleaseString);

    IceFlush (iceConn);

    iceConn->connection_status = IceConnectAccepted;
}



static void
AcceptProtocol (iceConn, hisOpcode, myOpcode, versionIndex, vendor, release)

IceConn iceConn;
int  	hisOpcode;
int  	myOpcode;
int  	versionIndex;
char 	*vendor;
char 	*release;

{
    iceProtocolReplyMsg	*pMsg;
    char		*pData;
    int			extra;

    extra = XPCS_BYTES (vendor) + XPCS_BYTES (release);

    IceGetHeaderExtra (iceConn, 0, ICE_ProtocolReply,
	SIZEOF (iceProtocolReplyMsg), WORD64COUNT (extra),
	iceProtocolReplyMsg, pMsg, pData);

    pMsg->protocolOpcode = myOpcode;
    pMsg->versionIndex = versionIndex;

    STORE_XPCS (pData, vendor);
    STORE_XPCS (pData, release);

    IceFlush (iceConn);


    /*
     * We may be using a different major opcode for this protocol
     * than the other client.  Whenever we get a message, we must
     * map to our own major opcode.
     */

    _IceAddOpcodeMapping (iceConn, hisOpcode, myOpcode);
}



static void
PingReply (iceConn)

IceConn iceConn;

{
    IceSimpleMessage (iceConn, 0, ICE_PingReply);
    IceFlush (iceConn);
}



static Bool
ProcessError (iceConn, swap, replyWait)

IceConn		 iceConn;
Bool		 swap;
IceReplyWaitInfo *replyWait;

{
    int		invokeHandler;
    Bool	errorReturned = False;
    iceErrorMsg *message;
    char 	*pData;

    IceReadCompleteMessage (iceConn,
	SIZEOF (iceErrorMsg), iceErrorMsg, message, pData);

    if (!replyWait ||
	message->offendingSequenceNum != replyWait->sequence_of_request)
    {
	invokeHandler = 1;
    }
    else
    {
	if (iceConn->connect_to_you &&
	    ((!iceConn->connect_to_you->auth_active &&
            message->offendingMinorOpcode == ICE_ConnectionSetup) ||
            (iceConn->connect_to_you->auth_active &&
	    message->offendingMinorOpcode == ICE_AuthReply)))
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);
	    char *errorStr = NULL;
	    char *prefix, *temp;

	    invokeHandler = 0;
	    errorReturned = True;

	    switch (message->errorClass)
	    {
	    case IceNoVersion:

		errorStr =
		"None of the ICE versions specified are supported";
		break;

	    case IceNoAuth:

		errorStr =
		"None of the authentication protocols specified are supported";
		break;

	    case IceAuthRejected:

		prefix = "Authentication Rejected, reason : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthFailed:

		prefix = "Authentication Failed, reason : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    default:
		invokeHandler = 1;
	    }

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = errorStr;
	}
	else if (iceConn->protosetup_to_you &&
	    ((!iceConn->protosetup_to_you->auth_active &&
            message->offendingMinorOpcode == ICE_ProtocolSetup) ||
            (iceConn->protosetup_to_you->auth_active &&
	    message->offendingMinorOpcode == ICE_AuthReply)))
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);
	    char *errorStr = "";
	    char *prefix, *temp;

	    invokeHandler = 0;
	    errorReturned = True;

	    switch (message->errorClass)
	    {
	    case IceNoVersion:

		errorStr =
		"None of the protocol versions specified are supported";
		break;

	    case IceNoAuth:

		errorStr =
		"None of the authentication protocols specified are supported";
		break;

	    case IceAuthRejected:

		prefix = "Authentication Rejected, reason : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthFailed:

		prefix = "Authentication Failed, reason : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceProtocolDuplicate:

		prefix = "Protocol was already registered : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceMajorOpcodeDuplicate:

		prefix = "The major opcode was already used : ";
		errorStr = (char *) malloc (strlen (prefix) + 2);
		sprintf (errorStr, "%s%d", prefix, (int) *pData);
		break;

	    case IceUnknownProtocol:

		prefix = "Unknown Protocol : ";
		EXTRACT_XPCS (pData, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    default:
		invokeHandler = 1;
	    }

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = errorStr;
	}

	if (errorReturned == True)
	{
	    /*
	     * If we tried to authenticate, tell the authentication
	     * procedure to clean up.
	     */

	    IceOCLauthProc authProc;

	    if (iceConn->connect_to_you &&
		iceConn->connect_to_you->auth_active)
	    {
		authProc = _IceOCLauthRecs[
		    iceConn->connect_to_you->my_auth_index].auth_proc;

		(*authProc) (&iceConn->connect_to_you->my_auth_state,
		    1 /* clean up */, 0, NULL, NULL, NULL, NULL);
	    }
	    else if (iceConn->protosetup_to_you &&
		iceConn->protosetup_to_you->auth_active)
	    {
		_IceOCLprotocol *protocol = _IceProtocols[
		    iceConn->protosetup_to_you->my_opcode - 1].orig_client;

		authProc = protocol->auth_recs[iceConn->
		    protosetup_to_you->my_auth_index].auth_proc;

		(*authProc) (&iceConn->protosetup_to_you->my_auth_state,
		    1 /* clean up */, 0, NULL, NULL, NULL, NULL);
	    }
	}
    }

    if (invokeHandler)
    {
	(*_IceErrorHandler) (iceConn, message->offendingMinorOpcode,
	    message->offendingSequenceNum, message->errorClass,
	    message->severity, (IcePointer) pData);
    }

    return (errorReturned);
}



static void
ProcessConnectionSetup (iceConn, swap)

IceConn			iceConn;
Bool			swap;

{
    iceConnectionSetupMsg *message;
    int  myVersionCount, hisVersionCount;
    int	 myVersionIndex, hisVersionIndex;
    int  hisMajorVersion, hisMinorVersion;
    int	 myAuthCount, hisAuthCount;
    int	 found, i, j;
    char *myAuthName, **hisAuthNames;
    char *pData;
    char *vendor = NULL;
    char *release = NULL;
    int  accept_setup_now = 0;

    IceReadCompleteMessage (iceConn, SIZEOF (iceConnectionSetupMsg),
	iceConnectionSetupMsg, message, pData);

    EXTRACT_XPCS (pData, vendor);
    EXTRACT_XPCS (pData, release);

    if ((hisAuthCount = message->authCount) > 0)
    {
	hisAuthNames = (char **) malloc (hisAuthCount * sizeof (char *));
	EXTRACT_LISTOF_XPCS (pData, hisAuthCount, hisAuthNames);
    }

    hisVersionCount = message->versionCount;
    myVersionCount = _IceVersionCount;

    hisVersionIndex = myVersionIndex = found = 0;

    for (i = 0; i < hisVersionCount && !found; i++)
    {
	EXTRACT_CARD8 (pData, hisMajorVersion);
	EXTRACT_CARD8 (pData, hisMinorVersion);

	for (j = 0; j < myVersionCount && !found; j++)
	{
	    if (_IceVersions[j].major_version == hisMajorVersion &&
		_IceVersions[j].minor_version == hisMinorVersion)
	    {
		hisVersionIndex = i;
		myVersionIndex = j;
		found = 1;
	    }
	}
    }

    if (!found)
    {
	_IceErrorNoVersion (iceConn, ICE_ConnectionSetup);
	iceConn->connection_status = IceConnectRejected;

	free (vendor);
	free (release);

	if (hisAuthCount > 0)
	{
	    for (i = 0; i < hisAuthCount; i++)
		free (hisAuthNames[i]);
	
	    free ((char *) hisAuthNames);
	}

	return;
    }

    if ((myAuthCount = _IceAuthCount) < 1)
    {
	/*
	 * No Authentication for this Connection Setup.  Simple accept.
	 */

	accept_setup_now = 1;
    }
    else
    {
	int myAuthIndex = 0;
	int hisAuthIndex = 0;

	for (i = found = 0; i < myAuthCount && !found; i++)
	{
	    myAuthName = _IceACLauthRecs[i].auth_name;

	    for (j = 0; j < hisAuthCount && !found; j++)
		if (strcmp (myAuthName, hisAuthNames[j]) == 0)
		{
		    myAuthIndex = i;
		    hisAuthIndex = j;
		    found = 1;
		}
	}

	if (!found)
	{
	    _IceErrorNoAuthentication (iceConn, ICE_ConnectionSetup);
	    iceConn->connection_status = IceConnectRejected;

	    free (vendor);
	    free (release);
	}
	else
	{
	    IceACLauthStatus	status;
	    int			authDataLen;
	    IcePointer		authData = NULL;
	    IcePointer		authState;
	    char		*errorString = NULL;
	    IceACLauthProc	authProc =
		_IceACLauthRecs[myAuthIndex].auth_proc;

	    authState = NULL;

	    status = (*authProc) (&authState, 0, NULL,
		&authDataLen, &authData, &errorString);

	    if (status == IceACLauthContinue)
	    {
		_IceConnectToMeInfo *setupInfo;

		AuthRequired (iceConn, hisAuthIndex, authDataLen, authData);

		iceConn->connect_to_me = setupInfo = (_IceConnectToMeInfo *)
		    malloc (sizeof (_IceConnectToMeInfo));

		setupInfo->my_version_index = myVersionIndex;
		setupInfo->his_version_index = hisVersionIndex;
		setupInfo->his_vendor = vendor;
		setupInfo->his_release = release;
		setupInfo->my_auth_index = myAuthIndex;
		setupInfo->my_auth_state = authState;
	    }
	    else if (status == IceACLauthAccepted)
	    {
		accept_setup_now = 1;
	    }

	    if (authData && authDataLen > 0)
		free ((char *) authData);

	    if (errorString)
		free (errorString);
	}
    }
    
    if (accept_setup_now)
    {
	AcceptConnection (iceConn, hisVersionIndex);

	iceConn->vendor = vendor;
	iceConn->release = release;
	iceConn->my_ice_version_index = myVersionIndex;
    }

    if (hisAuthCount > 0)
    {
	for (i = 0; i < hisAuthCount; i++)
	    free (hisAuthNames[i]);
	
	free ((char *) hisAuthNames);
    }
}



static Bool
ProcessAuthRequired (iceConn, swap, replyWait)

IceConn			iceConn;
Bool			swap;
IceReplyWaitInfo	*replyWait;

{
    iceAuthRequiredMsg  *message;
    int			authDataLen;
    IcePointer 		authData;
    int 		replyDataLen;
    IcePointer 		replyData = NULL;
    char		*errorString = NULL;
    IceOCLauthProc	authProc;
    IceOCLauthStatus	status;
    IcePointer 		authState;
    int			realAuthIndex;

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthRequiredMsg),
	iceAuthRequiredMsg, message, authData);

    if (iceConn->connect_to_you)
    {
	if ((int) message->authIndex >= _IceAuthCount)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    errorString = "Received bad authIndex in the AuthRequired message";

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = errorString;

	    _IceErrorAuthenticationFailed (iceConn,
		ICE_AuthRequired, errorString);

	    return (1);
	}
	else
	{
	    authProc = _IceOCLauthRecs[message->authIndex].auth_proc;

	    iceConn->connect_to_you->auth_active = 1;
	}
    }
    else if (iceConn->protosetup_to_you)
    {
	if ((int) message->authIndex >=
	    iceConn->protosetup_to_you->my_auth_count)
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    errorString = "Received bad authIndex in the AuthRequired message";

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = errorString;

	    _IceErrorAuthenticationFailed (iceConn,
		ICE_AuthRequired, errorString);

	    return (1);
	}
	else
	{
	    _IceOCLprotocol *myProtocol = _IceProtocols[
	        iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	    realAuthIndex = iceConn->protosetup_to_you->
		my_auth_indices[message->authIndex];

	    authProc = myProtocol->auth_recs[realAuthIndex].auth_proc;

	    iceConn->protosetup_to_you->auth_active = 1;
	}
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, ICE_AuthRequired, IceCanContinue);
	return (0);
    }

    authState = NULL;
    authDataLen = message->length << 3;

    status = (*authProc) (&authState, 0 /* don't clean up */,
	authDataLen, authData, &replyDataLen, &replyData, &errorString);

    if (status == IceOCLauthHaveReply)
    {
	AuthReply (iceConn, replyDataLen, replyData);

	replyWait->sequence_of_request = iceConn->sequence;
	replyWait->minor_opcode_of_request = ICE_AuthReply;

	if (iceConn->connect_to_you)
	{
	    iceConn->connect_to_you->my_auth_state = authState;
	    iceConn->connect_to_you->my_auth_index = message->authIndex;
	}
	else if (iceConn->protosetup_to_you)
	{
	    iceConn->protosetup_to_you->my_auth_state = authState;
	    iceConn->protosetup_to_you->my_auth_index = realAuthIndex;
	}
    }
    else if (status == IceOCLauthRejected || status == IceOCLauthFailed)
    {
	char *prefix, *returnErrorString;

	if (status == IceOCLauthRejected)
	{
	    _IceErrorAuthenticationRejected (iceConn,
	        ICE_AuthRequired, errorString);

	    prefix = "Authentication Rejected, reason : ";
	}
	else
	{
	    _IceErrorAuthenticationFailed (iceConn,
	       ICE_AuthRequired, errorString);

	    prefix = "Authentication Failed, reason : ";
	}

	returnErrorString = (char *) malloc (strlen (prefix) +
	    strlen (errorString) + 1);
	sprintf (returnErrorString, "%s%s", prefix, errorString);
	free (errorString);
	
	if (iceConn->connect_to_you)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = returnErrorString;
	}
	else
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = returnErrorString;
	}
    }

    if (replyData && replyDataLen > 0)
	free ((char *) replyData);

    return (status != IceOCLauthHaveReply);
}



static void
ProcessAuthReply (iceConn, swap)

IceConn		iceConn;
Bool		swap;

{
    iceAuthReplyMsg 	*message;
    int			replyDataLen;
    IcePointer		replyData;
    int 		authDataLen;
    IcePointer 		authData = NULL;
    char		*errorString = NULL;

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthReplyMsg),
	iceAuthReplyMsg, message, replyData);

    replyDataLen = message->length << 3;

    if (iceConn->connect_to_me)
    {
	IceACLauthProc authProc = _IceACLauthRecs[
	    iceConn->connect_to_me->my_auth_index].auth_proc;
	IceACLauthStatus status =
	    (*authProc) (&iceConn->connect_to_me->my_auth_state,
	    replyDataLen, replyData, &authDataLen, &authData, &errorString);

	if (status == IceACLauthContinue)
	{
	    AuthNextPhase (iceConn, authDataLen, authData);
	}
	else if (status == IceACLauthAccepted)
	{
	    AcceptConnection (iceConn,
		iceConn->connect_to_me->his_version_index);

	    iceConn->vendor = iceConn->connect_to_me->his_vendor;
	    iceConn->release = iceConn->connect_to_me->his_release;
	    iceConn->my_ice_version_index =
		iceConn->connect_to_me->my_version_index;

	    free ((char *) iceConn->connect_to_me);
	    iceConn->connect_to_me = NULL;
	}
	else if (status == IceACLauthRejected || status == IceACLauthFailed)
	{
	    free (iceConn->connect_to_me->his_vendor);
	    free (iceConn->connect_to_me->his_release);
	    free ((char *) iceConn->connect_to_me);
	    iceConn->connect_to_me = NULL;

	    if (status == IceACLauthRejected)
	    {
		_IceErrorAuthenticationRejected (iceConn,
	            ICE_AuthReply, errorString);
	    }
	    else
	    {
		_IceErrorAuthenticationFailed (iceConn,
	            ICE_AuthReply, errorString);
	    }
	}
    }
    else if (iceConn->protosetup_to_me)
    {
	_IceACLprotocol *myProtocol = _IceProtocols[iceConn->protosetup_to_me->
	    my_opcode - 1].accept_client;
	IceACLauthProc authProc = myProtocol->auth_recs[
	    iceConn->protosetup_to_me->my_auth_index].auth_proc;
	IceACLauthStatus status =
	    (*authProc) (&iceConn->protosetup_to_me->my_auth_state,
	    replyDataLen, replyData, &authDataLen, &authData, &errorString);
	int free_setup_info = 1;

	if (status == IceACLauthContinue)
	{
	    AuthNextPhase (iceConn, authDataLen, authData);
	    free_setup_info = 0;
	}
	else if (status == IceACLauthAccepted)
	{
	    IceACLprocessMsgCB		processMsgCB;
	    IceProtocolSetupNotifyCB	protocolSetupNotifyCB;

	    processMsgCB = myProtocol->version_recs[
	        iceConn->protosetup_to_me->my_version_index].process_msg_cb;

	    AcceptProtocol (iceConn,
	        iceConn->protosetup_to_me->his_opcode,
	        iceConn->protosetup_to_me->my_opcode,
	        iceConn->protosetup_to_me->his_version_index,
		myProtocol->vendor, myProtocol->release);

	    iceConn->process_msg_info[iceConn->protosetup_to_me->his_opcode -
	        iceConn->his_min_opcode].accept_flag = 1;

	    iceConn->process_msg_info[iceConn->protosetup_to_me->his_opcode -
	        iceConn->his_min_opcode].process_msg_cb.
                accept_client = processMsgCB;

	    protocolSetupNotifyCB = myProtocol->protocol_setup_notify_cb;

	    if (protocolSetupNotifyCB)
	    {
		(*protocolSetupNotifyCB) (iceConn,
		    myProtocol->version_recs[iceConn->protosetup_to_me->
		        my_version_index].major_version,
		    myProtocol->version_recs[iceConn->protosetup_to_me->
		        my_version_index].minor_version,
		    iceConn->protosetup_to_me->his_vendor,
		    iceConn->protosetup_to_me->his_release);
	    }
	}
	else if (status == IceACLauthRejected)
	{
	    _IceErrorAuthenticationRejected (iceConn,
	        ICE_AuthReply, errorString);
	}
	else if (status == IceACLauthFailed)
	{
	    _IceErrorAuthenticationFailed (iceConn,
	        ICE_AuthReply, errorString);
	}

	if (free_setup_info)
	{
	    free (iceConn->protosetup_to_me->his_vendor);
	    free (iceConn->protosetup_to_me->his_release);
	    free ((char *) iceConn->protosetup_to_me);
	    iceConn->protosetup_to_me = NULL;
	}
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, ICE_AuthReply, IceCanContinue);
    }

    if (authData && authDataLen > 0)
	free ((char *) authData);

    if (errorString)
	free (errorString);
}



static Bool
ProcessAuthNextPhase (iceConn, swap, replyWait)

IceConn		  	iceConn;
Bool			swap;
IceReplyWaitInfo	*replyWait;

{
    iceAuthNextPhaseMsg *message;
    int 		authDataLen;
    IcePointer		authData;
    int 		replyDataLen;
    IcePointer		replyData = NULL;
    char		*errorString = NULL;
    IceOCLauthProc 	authProc;
    IceOCLauthStatus	status;
    IcePointer 		*authState;

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthNextPhaseMsg),
	iceAuthNextPhaseMsg, message, authData);

    if (iceConn->connect_to_you)
    {
	authProc = _IceOCLauthRecs[
	    iceConn->connect_to_you->my_auth_index].auth_proc;

	authState = &iceConn->connect_to_you->my_auth_state;
    }
    else if (iceConn->protosetup_to_you)
    {
	_IceOCLprotocol *myProtocol =
	  _IceProtocols[iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	authProc = myProtocol->auth_recs[
	    iceConn->protosetup_to_you->my_auth_index].auth_proc;

	authState = &iceConn->protosetup_to_you->my_auth_state;
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, ICE_AuthNextPhase, IceCanContinue);
	return (0);
    }

    authDataLen = message->length << 3;

    status = (*authProc) (authState, 0 /* don't clean up */,
	authDataLen, authData, &replyDataLen, &replyData, &errorString);

    if (status == IceOCLauthHaveReply)
    {
	AuthReply (iceConn, replyDataLen, replyData);

	replyWait->sequence_of_request = iceConn->sequence;
    }
    else if (status == IceOCLauthRejected || status == IceOCLauthFailed)
    {
	char *prefix, *returnErrorString;

	if (status == IceOCLauthRejected)
	{
	    _IceErrorAuthenticationRejected (iceConn,
	       ICE_AuthNextPhase, errorString);

	    prefix = "Authentication Rejected, reason : ";
	}
	else if (status == IceOCLauthFailed)
	{
	    _IceErrorAuthenticationFailed (iceConn,
	       ICE_AuthNextPhase, errorString);

	    prefix = "Authentication Failed, reason : ";
	}

	returnErrorString = (char *) malloc (strlen (prefix) +
	    strlen (errorString) + 1);
	sprintf (returnErrorString, "%s%s", prefix, errorString);
	free (errorString);

	if (iceConn->connect_to_you)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = returnErrorString;
	}
	else
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = returnErrorString;
	}
    }

    if (replyData && replyDataLen > 0)
	free ((char *) replyData);

    return (status != IceOCLauthHaveReply);
}



static Bool
ProcessConnectionReply (iceConn, swap, replyWait)

IceConn			iceConn;
Bool			swap;
IceReplyWaitInfo 	*replyWait;

{
    iceConnectionReplyMsg 	*message;
    char 			*pData;

    IceReadCompleteMessage (iceConn, SIZEOF (iceConnectionReplyMsg),
	iceConnectionReplyMsg, message, pData);

    if (iceConn->connect_to_you)
    {
	_IceReply *reply = (_IceReply *) (replyWait->reply);

	/*
	 * If authentication took place, we're done.
	 * Tell the authentication procedure to clean up.
	 */

	if (iceConn->connect_to_you->auth_active)
	{
	    IceOCLauthProc authProc = _IceOCLauthRecs[
		iceConn->connect_to_you->my_auth_index].auth_proc;

	    (*authProc) (&iceConn->connect_to_you->my_auth_state,
		1 /* clean up */,
	        0, NULL, NULL, NULL, NULL);
	}


	/*
	 * Now fill in the connection reply message.
	 */

	reply->type = ICE_CONNECTION_REPLY;
        reply->connection_reply.version_index = message->versionIndex;

	EXTRACT_XPCS (pData, reply->connection_reply.vendor);
	EXTRACT_XPCS (pData, reply->connection_reply.release);

	return (1);
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, ICE_ConnectionReply, IceCanContinue);
	return (0);
    }
}



static void
ProcessProtocolSetup (iceConn, swap)

IceConn			iceConn;
Bool			swap;

{
    iceProtocolSetupMsg	*message;
    _IceACLprotocol 	*myProtocol;
    int  	      	myVersionCount, hisVersionCount;
    int	 	      	myVersionIndex, hisVersionIndex;
    int  	      	hisMajorVersion, hisMinorVersion;
    int	 	      	myAuthCount, hisAuthCount;
    int  	      	myOpcode, hisOpcode;
    int	 	      	found, i, j;
    char	      	*myAuthName, **hisAuthNames;
    char 	      	*protocolName;
    char 		*pData;
    char 	      	*vendor = NULL;
    char 	      	*release = NULL;
    int  	      	accept_setup_now = 0;

    IceReadCompleteMessage (iceConn, SIZEOF (iceProtocolSetupMsg),
	iceProtocolSetupMsg, message, pData);

    if (iceConn->process_msg_info && iceConn->process_msg_info[
	message->protocolOpcode - iceConn->his_min_opcode].in_use)
    {
	_IceErrorMajorOpcodeDuplicate (iceConn, message->protocolOpcode);
	return;
    }

    EXTRACT_XPCS (pData, protocolName);

    if (iceConn->process_msg_info)
    {
	for (i = 0;
	    i <= (iceConn->his_max_opcode - iceConn->his_min_opcode); i++)
	{
	    if (iceConn->process_msg_info[i].in_use && strcmp (protocolName,
	        iceConn->process_msg_info[i].protocol->protocol_name) == 0)
	    {
		_IceErrorProtocolDuplicate (iceConn, protocolName);
		free (protocolName);
		return;
	    }
	}
    }

    for (i = 0; i < _IceLastMajorOpcode; i++)
	if (strcmp (protocolName, _IceProtocols[i].protocol_name) == 0)
	    break;

    if (i < _IceLastMajorOpcode &&
        (myProtocol = _IceProtocols[i].accept_client) != NULL)
    {
	hisOpcode = message->protocolOpcode;
	myOpcode = i + 1;
	free (protocolName);
    }
    else
    {
	_IceErrorUnknownProtocol (iceConn, protocolName);
	free (protocolName);
	return;
    }

    EXTRACT_XPCS (pData, vendor);
    EXTRACT_XPCS (pData, release);

    if ((hisAuthCount = message->authCount) > 0)
    {
	hisAuthNames = (char **) malloc (hisAuthCount * sizeof (char *));
	EXTRACT_LISTOF_XPCS (pData, hisAuthCount, hisAuthNames);
    }

    hisVersionCount = message->versionCount;
    myVersionCount = myProtocol->version_count;

    hisVersionIndex = myVersionIndex = found = 0;

    for (i = 0; i < hisVersionCount && !found; i++)
    {
	EXTRACT_CARD8 (pData, hisMajorVersion);
	EXTRACT_CARD8 (pData, hisMinorVersion);

	for (j = 0; j < myVersionCount && !found; j++)
	{
	    if (myProtocol->version_recs[j].major_version == hisMajorVersion &&
		myProtocol->version_recs[j].minor_version == hisMinorVersion)
	    {
		hisVersionIndex = i;
		myVersionIndex = j;
		found = 1;
	    }
	}
    }

    if (!found)
    {
	_IceErrorNoVersion (iceConn, ICE_ProtocolSetup);

	free (vendor);
	free (release);

	if (hisAuthCount > 0)
	{
	    for (i = 0; i < hisAuthCount; i++)
		free (hisAuthNames[i]);
	
	    free ((char *) hisAuthNames);
	}

	return;
    }

    if ((myAuthCount = myProtocol->auth_count) < 1)
    {
	/*
	 * No Authentication for this protocol.  Simple accept.
	 */

	accept_setup_now = 1;
    }
    else
    {
	int myAuthIndex = 0;
	int hisAuthIndex = 0;

	for (i = found = 0; i < myAuthCount && !found; i++)
	{
	    myAuthName = myProtocol->auth_recs[i].auth_name;

	    for (j = 0; j < hisAuthCount && !found; j++)
		if (strcmp (myAuthName, hisAuthNames[j]) == 0)
		{
		    myAuthIndex = i;
		    hisAuthIndex = j;
		    found = 1;
		}
	}

	if (!found)
	{
	    _IceErrorNoAuthentication (iceConn, ICE_ProtocolSetup);
	}
	else
	{
	    IceACLauthStatus   	status;
	    int 		authDataLen;
	    IcePointer 		authData = NULL;
	    IcePointer 		authState;
	    char		*errorString = NULL;
	    IceACLauthProc	authProc =
		myProtocol->auth_recs[myAuthIndex].auth_proc;

	    authState = NULL;

	    status = (*authProc) (&authState, 0, NULL,
	        &authDataLen, &authData, &errorString);

	    if (status == IceACLauthContinue)
	    {
		_IceProtoSetupToMeInfo *setupInfo;

		AuthRequired (iceConn, hisAuthIndex, authDataLen, authData);
	 
		iceConn->protosetup_to_me = setupInfo =
		    (_IceProtoSetupToMeInfo *) malloc (
		    sizeof (_IceProtoSetupToMeInfo));

		setupInfo->his_opcode = hisOpcode;
		setupInfo->my_opcode = myOpcode;
		setupInfo->my_version_index = myVersionIndex;
		setupInfo->his_version_index = hisVersionIndex;
		setupInfo->his_vendor = vendor;
		setupInfo->his_release = release;
		vendor = release = NULL;   /* so we don't free it */
		setupInfo->my_auth_index = myAuthIndex;
		setupInfo->my_auth_state = authState;
	    }
	    else if (status == IceACLauthAccepted)
	    {
		accept_setup_now = 1;
	    }

	    if (authData && authDataLen > 0)
		free ((char *) authData);

	    if (errorString)
		free (errorString);
	}
    }


    if (accept_setup_now)
    {
	IceACLprocessMsgCB		processMsgCB;
	IceProtocolSetupNotifyCB	protocolSetupNotifyCB;

	processMsgCB = myProtocol->version_recs[
	    myVersionIndex].process_msg_cb;

	AcceptProtocol (iceConn, hisOpcode, myOpcode, hisVersionIndex,
	    myProtocol->vendor, myProtocol->release);

	iceConn->process_msg_info[hisOpcode -
	    iceConn->his_min_opcode].accept_flag = 1;

	iceConn->process_msg_info[hisOpcode -
	    iceConn->his_min_opcode].process_msg_cb.
	    accept_client = processMsgCB;

	protocolSetupNotifyCB = myProtocol->protocol_setup_notify_cb;

	if (protocolSetupNotifyCB)
	{
	    (*protocolSetupNotifyCB) (iceConn,
		myProtocol->version_recs[myVersionIndex].major_version,
		myProtocol->version_recs[myVersionIndex].minor_version,
	        vendor, release);
	}
    }

    if (vendor)
	free (vendor);

    if (release)
	free (release);

    if (hisAuthCount > 0)
    {
	for (i = 0; i < hisAuthCount; i++)
	    free (hisAuthNames[i]);

	free ((char *) hisAuthNames);
    }
}



static Bool
ProcessProtocolReply (iceConn, swap, replyWait)

IceConn		  	iceConn;
Bool			swap;
IceReplyWaitInfo 	*replyWait;

{
    iceProtocolReplyMsg *message;
    char		*pData;

    IceReadCompleteMessage (iceConn, SIZEOF (iceProtocolReplyMsg),
	iceProtocolReplyMsg, message, pData);

    if (iceConn->protosetup_to_you)
    {
	_IceProtocolReply *reply = 
	    &(((_IceReply *) (replyWait->reply))->protocol_reply);

	/*
	 * If authentication took place, we're done.
	 * Tell the authentication procedure to clean up.
	 */

	if (iceConn->protosetup_to_you->auth_active)
	{
	    _IceOCLprotocol *myProtocol = _IceProtocols[
		iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	    IceOCLauthProc authProc = myProtocol->auth_recs[
		iceConn->protosetup_to_you->my_auth_index].auth_proc;

	    (*authProc) (&iceConn->protosetup_to_you->my_auth_state,
		1 /* clean up */,
	        0, NULL, NULL, NULL, NULL);
	}


	/*
	 * Now fill in the protocol reply message.
	 */

	reply->type = ICE_PROTOCOL_REPLY;
	reply->major_opcode = message->protocolOpcode;
        reply->version_index = message->versionIndex;

	EXTRACT_XPCS (pData, reply->vendor);
	EXTRACT_XPCS (pData, reply->release);

	return (1);
    }
    else
    {
	_IceErrorBadState (iceConn, ICE_ProtocolReply, IceCanContinue);
	return (0);
    }
}



Bool
_IceProcessCoreMessage (iceConn, opcode, swap, replyWait)

IceConn 	 iceConn;
int     	 opcode;
Bool    	 swap;
IceReplyWaitInfo *replyWait;

{
    Bool replyReady = False;

    switch (opcode)
    {
    case ICE_Error:

	replyReady = ProcessError (iceConn, swap, replyWait);
	break;

    case ICE_ConnectionSetup:

	ProcessConnectionSetup (iceConn, swap);
	break;

    case ICE_AuthRequired:

	replyReady = ProcessAuthRequired (iceConn, swap, replyWait);
        break;

    case ICE_AuthReply:

	ProcessAuthReply (iceConn, swap);
	break;

    case ICE_AuthNextPhase:

	replyReady = ProcessAuthNextPhase (iceConn, swap, replyWait);
	break;

    case ICE_ConnectionReply:

	replyReady = ProcessConnectionReply (iceConn, swap, replyWait);
	break;

    case ICE_ProtocolSetup:

	ProcessProtocolSetup (iceConn, swap);
	break;

    case ICE_ProtocolReply:

	replyReady = ProcessProtocolReply (iceConn, swap, replyWait);
	break;

    case ICE_Ping:

	PingReply (iceConn);
	break;

    case ICE_PingReply:

	if (iceConn->ping_waits)
	{
	    _IcePingWait *next = iceConn->ping_waits->next;

	    (*iceConn->ping_waits->ping_reply_cb) (iceConn,
		iceConn->ping_waits->client_data);

	    free ((char *) iceConn->ping_waits);
	    iceConn->ping_waits = next;
	}
	else
	{
	    _IceErrorBadState (iceConn, ICE_PingReply, IceCanContinue);
	}
	break;

    case ICE_WantToClose:

	break;

    case ICE_NoClose:

	break;

    default:

	_IceErrorBadMinor (iceConn, opcode, IceCanContinue);
	break;
    }

    return (replyReady);
}
