/* $XConsortium: protosetup.c,v 1.2 93/08/26 17:10:08 mor Exp $ */
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


Status
IceProtocolSetup (iceConn, myOpcode, authCount, authIndices,
    majorVersionRet, minorVersionRet, vendorRet, releaseRet,
    errorLength, errorStringRet)

IceConn	iceConn;
int 	myOpcode;
int	authCount;
int	*authIndices;
int	*majorVersionRet;
int	*minorVersionRet;
char	**vendorRet;
char	**releaseRet;
int  	errorLength;
char 	*errorStringRet;

{
    iceProtocolSetupMsg	*pMsg;
    char		*pData;
    _IceProtocol	*myProtocol;
    int			extra;
    Bool		gotReply;
    int			accepted, i;
    int			hisOpcode;
    int			doFavor;
    unsigned long	setup_sequence;
    IceReplyWaitInfo 	replyWait;
    _IceReply		reply;
    IceOCLversionRec	*versionRec;

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    *majorVersionRet = 0;
    *minorVersionRet = 0;
    *vendorRet = NULL;
    *releaseRet = NULL;

    if (myOpcode < 1 || myOpcode > _IceLastMajorOpcode)
    {
	strncpy (errorStringRet, "myOpcode out of range", errorLength);
	return (0);
    }

    myProtocol = &_IceProtocols[myOpcode - 1];

    if (myProtocol->orig_client == NULL)
    {
	strncpy (errorStringRet,
	    "IceRegisterForProtocolSetup was not called", errorLength);
	return (0);
    }
    else if (authCount > myProtocol->orig_client->auth_count)
    {
	strncpy (errorStringRet, "authCount is bad", errorLength);
	return (0);
    }


    if (!(doFavor = (authCount > 0 && authIndices == NULL)))
	for (i = 0; i < authCount; i++)
	    if (authIndices[i] >= myProtocol->orig_client->auth_count)
	    {
		strncpy (errorStringRet,
	            "Bad authentication indices", errorLength);
		return (0);
	    }

    extra = XPCS_BYTES (myProtocol->protocol_name) +
            XPCS_BYTES (myProtocol->orig_client->vendor) +
            XPCS_BYTES (myProtocol->orig_client->release);
    for (i = 0; i < authCount; i++)
    {
	extra += XPCS_BYTES (myProtocol->orig_client->auth_recs[
	    doFavor ? i : authIndices[i]].auth_name);
    }
    extra += (myProtocol->orig_client->version_count * 2);

    IceGetHeaderExtra (iceConn, 0, ICE_ProtocolSetup,
	SIZEOF (iceProtocolSetupMsg), WORD64COUNT (extra),
	iceProtocolSetupMsg, pMsg, pData);

    setup_sequence = iceConn->sequence;

    pMsg->protocolOpcode = myOpcode;
    pMsg->versionCount = myProtocol->orig_client->version_count;
    pMsg->authCount = authCount;

    STORE_XPCS (pData, myProtocol->protocol_name);
    STORE_XPCS (pData, myProtocol->orig_client->vendor);
    STORE_XPCS (pData, myProtocol->orig_client->release);

    for (i = 0; i < authCount; i++)
    {
	STORE_XPCS (pData, myProtocol->orig_client->auth_recs[
	    doFavor ? i : authIndices[i]].auth_name);
    }

    for (i = 0; i < myProtocol->orig_client->version_count; i++)
    {
	STORE_CARD8 (pData,
	    myProtocol->orig_client->version_recs[i].major_version);
	STORE_CARD8 (pData,
	    myProtocol->orig_client->version_recs[i].minor_version);
    }

    IceFlush (iceConn);


    /*
     * Process messages until we get a Protocol Reply.
     */

    replyWait.sequence_of_request = setup_sequence;
    replyWait.major_opcode_of_request = 0;
    replyWait.minor_opcode_of_request = ICE_ProtocolSetup;
    replyWait.reply = (IcePointer) &reply;

    iceConn->protosetup_to_you = (_IceProtoSetupToYouInfo *) malloc (
	sizeof (_IceProtoSetupToYouInfo));
    iceConn->protosetup_to_you->my_opcode = myOpcode;
    iceConn->protosetup_to_you->my_auth_count = authCount;
    iceConn->protosetup_to_you->auth_active = 0;

    if (authCount > 0)
    {
	iceConn->protosetup_to_you->my_auth_indices = (int *) malloc (
	    authCount * sizeof (int));

	if (doFavor)
	{
	    for (i = 0; i < authCount; i++)
		iceConn->protosetup_to_you->my_auth_indices[i] = i;
	}
	else
	{
	    bcopy (authIndices, iceConn->protosetup_to_you->my_auth_indices,
	        authCount * sizeof (int));
	}
    }
    else
	iceConn->protosetup_to_you->my_auth_indices = NULL;

    gotReply = False;
    accepted = 0;

    while (gotReply == False)
	if ((gotReply = IceProcessMessage (iceConn, &replyWait)) == True)
	{
	    if (reply.type == ICE_PROTOCOL_REPLY)
	    {
		if (reply.protocol_reply.version_index >=
		    myProtocol->orig_client->version_count)
		{
		    strncpy (errorStringRet,
	                "Got a bad version index in the Protocol Reply",
		        errorLength);

		    free (reply.protocol_reply.vendor);
		    free (reply.protocol_reply.release);
		}
		else
		{
		    versionRec = &(myProtocol->orig_client->version_recs[
		        reply.protocol_reply.version_index]);

		    accepted = 1;
		}
	    }
	    else /* reply.type == ICE_PROTOCOL_ERROR */
	    {
		/* Protocol Setup failed */
		
		strncpy (errorStringRet, reply.protocol_error.error_message,
		    errorLength);

		free (reply.protocol_error.error_message);
	    }

	    if (iceConn->protosetup_to_you->my_auth_indices)
		free ((char *) iceConn->protosetup_to_you->my_auth_indices);
	    free ((char *) iceConn->protosetup_to_you);
	    iceConn->protosetup_to_you = NULL;
	}

    if (accepted)
    {
	*majorVersionRet = versionRec->major_version;
	*minorVersionRet = versionRec->minor_version;
	*vendorRet = reply.protocol_reply.vendor;
	*releaseRet = reply.protocol_reply.release;
	

	/*
	 * Increase the reference count for the number of active protocols.
	 */

	iceConn->proto_ref_count++;


	/*
	 * We may be using a different major opcode for this protocol
	 * than the other client.  Whenever we get a message, we must
	 * map to our own major opcode.
	 */

	hisOpcode = reply.protocol_reply.major_opcode;

	_IceAddOpcodeMapping (iceConn, hisOpcode, myOpcode);

	iceConn->process_msg_info[hisOpcode -
	    iceConn->his_min_opcode].accept_flag = 0;

	iceConn->process_msg_info[hisOpcode -
	    iceConn->his_min_opcode].process_msg_cb.orig_client =
		versionRec->process_msg_cb;

	return (1);
    }
    else
    {
	return (0);
    }
}
