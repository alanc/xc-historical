/* $XConsortium: sm_manager.c,v 1.4 93/09/12 16:22:40 mor Exp $ */
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


Status
SmsInitialize (vendor, release, callbacks)

char 		*vendor;
char 		*release;
SmsCallbacks 	*callbacks;

{
    if (!callbacks || !callbacks->new_client ||
        !callbacks->register_client || !callbacks->interact_request ||
        !callbacks->interact_done || !callbacks->save_yourself_done ||
        !callbacks->close_connection || !callbacks->set_properties ||
        !callbacks->get_properties)
    {
	/* We need callbacks!  Otherwise, we can't do anything */

	return (0);
    }

    if (!_SmsOpcode)
    {
	void _SmsProtocolSetupNotifyCB ();

	if ((_SmsOpcode = IceRegisterForProtocolReply ("XSMP",
	    vendor, release, _SmVersionCount, _SmsVersions,
	    _SmsProtocolSetupNotifyCB, _SmAuthCount, _SmsAuthRecs, NULL)) < 0)
	{
	   return (0);
	}
    }

    bcopy ((char *) callbacks,
	(char *) &_SmsCallbacks, sizeof (SmsCallbacks));

    return (1);
}



void
_SmsProtocolSetupNotifyCB (iceConn,
    majorVersion, minorVersion, vendor, release)

IceConn iceConn;
int	majorVersion;
int	minorVersion;
char  	*vendor;
char 	*release;

{
    SmsConn  	smsConn;

    smsConn = (SmsConn) malloc (sizeof (struct _SmsConn));

    smsConn->iceConn = iceConn;
    smsConn->proto_major_version = majorVersion;
    smsConn->proto_minor_version = minorVersion;
    smsConn->vendor = vendor;
    smsConn->release = release;
    smsConn->client_id = NULL;

    smsConn->save_yourself_in_progress = False;
    smsConn->waiting_to_interact = False;
    smsConn->interact_in_progress = False;

    _SmsConnectionObjs[_SmsConnectionCount++] = smsConn;


    /*
     * Now give the session manager the new smsConn
     */

    (*_SmsCallbacks.new_client) (smsConn, &smsConn->manager_data);
}



void
SmsRegisterClientReply (smsConn, clientId)

SmsConn smsConn;
char	*clientId;

{
    IceConn			iceConn = smsConn->iceConn;
    int				extra;
    smRegisterClientReplyMsg 	*pMsg;
    char 			*pData;

    extra = ARRAY8_BYTES (strlen (clientId));

    IceGetHeaderExtra (iceConn, _SmsOpcode, SM_RegisterClientReply,
	SIZEOF (smRegisterClientReplyMsg), WORD64COUNT (extra),
	smRegisterClientReplyMsg, pMsg, pData);

    STORE_ARRAY8 (pData, strlen (clientId), clientId);

    IceFlush (iceConn);

    smsConn->client_id = (char *) malloc (strlen (clientId) + 1);
    strcpy (smsConn->client_id, clientId);
}



void
SmsSaveYourself (smsConn, saveType, shutdown, interactStyle, fast)

SmsConn smsConn;
int	saveType;
Bool 	shutdown;
int	interactStyle;
Bool	fast;

{
    IceConn		iceConn = smsConn->iceConn;
    smSaveYourselfMsg	*pMsg;

    IceGetHeader (iceConn, _SmsOpcode, SM_SaveYourself,
	SIZEOF (smSaveYourselfMsg), smSaveYourselfMsg, pMsg);

    pMsg->saveType = saveType;
    pMsg->shutdown = shutdown;
    pMsg->interactStyle = interactStyle;
    pMsg->fast = fast;

    IceFlush (iceConn);

    smsConn->save_yourself_in_progress = True;
}



void
SmsInteract (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_Interact);
    IceFlush (iceConn);

    smsConn->waiting_to_interact = False;
    smsConn->interact_in_progress = True;
}



void
SmsDie (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_Die);
    IceFlush (iceConn);
}



void
SmsShutdownCancelled (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_ShutdownCancelled);
    IceFlush (iceConn);
}



void
SmsReturnProperties (smsConn, numProps, props)

SmsConn	smsConn;
int	numProps;
SmProp  *props;

{
    IceConn			iceConn = smsConn->iceConn;
    int 			bytes;
    smPropertiesReplyMsg	*pMsg;
    char 			*pBuf;
    char			*pStart;

    IceGetHeader (iceConn, _SmsOpcode, SM_PropertiesReply,
	SIZEOF (smPropertiesReplyMsg), smPropertiesReplyMsg, pMsg);

    LISTOF_PROP_BYTES (numProps, props, bytes);
    pMsg->length += WORD64COUNT (bytes);

    pBuf = pStart = IceAllocScratch (iceConn, bytes);

    STORE_LISTOF_PROPERTY (pBuf, numProps, props);

    IceWriteData (iceConn, bytes, pStart);
    IceFlush (iceConn);
}



void
SmsCleanUp (smsConn)

SmsConn smsConn;

{
    int i;

    for (i = 0; i < _SmsConnectionCount; i++)
	if (_SmsConnectionObjs[i] == smsConn)
	    break;

    if (i < _SmsConnectionCount)
    {
	IceProtocolShutdown (smsConn->iceConn, _SmsOpcode);

	if (i < _SmsConnectionCount - 1)
	{
	    _SmsConnectionObjs[i] =
		_SmsConnectionObjs[_SmsConnectionCount - 1];
	}

	_SmsConnectionCount--;

	if (smsConn->vendor)
	    free (smsConn->vendor);

	if (smsConn->release)
	    free (smsConn->release);

	if (smsConn->client_id)
	    free (smsConn->client_id);

	free ((char *) smsConn);
    }
}
