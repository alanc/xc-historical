/* $XConsortium: connect.c,v 1.24 94/03/15 13:37:23 mor Exp $ */
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

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>
#include <X11/Xtrans.h>
#include "globals.h"

static XtransConnInfo ConnectToPeer();

#ifndef X_NOT_STDC_ENV
#define Strstr strstr
#else
static char *Strstr(s1, s2)
    char *s1, *s2;
{
    int n1, n2;

    n1 = strlen(s1);
    n2 = strlen(s2);
    for ( ; n1 >= n2; s1++, n1--) {
	if (!strncmp(s1, s2, n2))
	    return s1;
    }	
    return NULL;
}
#endif

IceConn
IceOpenConnection (networkIdsList, mustAuthenticate, majorOpcodeCheck,
    errorLength, errorStringRet)

char *networkIdsList;
Bool mustAuthenticate;
int  majorOpcodeCheck;
int  errorLength;
char *errorStringRet;

{
    IceConn			iceConn;
    int				extra, i, j;
    int		       		endian;
    Bool			gotReply;
    unsigned long		setup_sequence;
    iceByteOrderMsg		*pByteOrderMsg;
    iceConnectionSetupMsg	*pSetupMsg;
    char			*pData;
    IceReplyWaitInfo 		replyWait;
    _IceReply		 	reply;
    int				authUsableCount;
    int				authUsableFlags[MAX_ICE_AUTH_NAMES];
    int				authIndices[MAX_ICE_AUTH_NAMES];

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    if (networkIdsList == NULL || *networkIdsList == '\0')
    {
	strncpy (errorStringRet,
	    "networkIdsList argument is NULL", errorLength);
	return (NULL);
    }

    /*
     * Check to see if we can use a previously created ICE connection.
     * If 'majorOpcodeCheck' is non-zero, it will contain a protocol major
     * opcode that we should make sure is not already active on the ICE
     * connection.  Some clients will want two seperate connections for the
     * same protocol to the same destination client.
     */

    for (i = 0; i < _IceConnectionCount; i++)
    {
	char *strptr;
	if ((strptr = (char *) Strstr (
	    networkIdsList, _IceConnectionStrings[i])) != NULL)
	{
	    char ch = *(strptr + strlen (_IceConnectionStrings[i]));
	    if (ch == ',' || ch == '\0')
	    {
		/*
		 * OK, we found a connection.  Now make sure the
		 * specified protocol is not already active on it.
		 */

		IceConn iceConn = _IceConnectionObjs[i];

		if (majorOpcodeCheck)
		{
		    for (j = iceConn->his_min_opcode;
		        j <= iceConn->his_max_opcode; j++)
		    {
			if (iceConn->process_msg_info[
			    j - iceConn->his_min_opcode].in_use &&
			    iceConn->process_msg_info[
			    j - iceConn->his_min_opcode].my_opcode ==
			    majorOpcodeCheck)
			    break;
		    }

		    if (j <= iceConn->his_max_opcode ||
			(iceConn->protosetup_to_you &&
			iceConn->protosetup_to_you->my_opcode ==
			majorOpcodeCheck))
		    {
			/* force a new connection to be created */
			break;
		    }
		}

		iceConn->open_ref_count++;
		return (iceConn);
	    }
	}
    }

    if ((iceConn = (IceConn) malloc (sizeof (struct _IceConn))) == NULL)
    {
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }


    /*
     * Open a network connection with the peer client.
     */

    if ((iceConn->trans_conn = ConnectToPeer (networkIdsList,
	&iceConn->connection_string)) == NULL)
    {
	free ((char *) iceConn);
	strncpy (errorStringRet, "Could not open network socket", errorLength);
	return (NULL);
    }

    iceConn->connection_status = IceConnectPending;
    iceConn->my_ice_version_index = 0;
    iceConn->send_sequence = 0;
    iceConn->receive_sequence = 0;

    if ((iceConn->inbuf = iceConn->inbufptr =
	(char *) malloc (ICE_INBUFSIZE)) == NULL)
    {
	_IceFreeConnection (iceConn, True);
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }

    iceConn->inbufmax = iceConn->inbuf + ICE_INBUFSIZE;

    if ((iceConn->outbuf = iceConn->outbufptr =
	(char *) malloc (ICE_OUTBUFSIZE)) == NULL)
    {
	_IceFreeConnection (iceConn, True);
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }

    iceConn->outbufmax = iceConn->outbuf + ICE_OUTBUFSIZE;

    iceConn->scratch = NULL;
    iceConn->scratch_size = 0;

    iceConn->process_msg_info = NULL;

    iceConn->listen_obj = NULL;

    iceConn->open_ref_count = 1;
    iceConn->proto_ref_count = 0;

    iceConn->skip_want_to_close = False;
    iceConn->want_to_close = False;

    iceConn->saved_reply_waits = NULL;
    iceConn->ping_waits = NULL;

    iceConn->connect_to_you = (_IceConnectToYouInfo *) malloc (
	sizeof (_IceConnectToYouInfo));
    iceConn->connect_to_you->auth_active = 0;
    iceConn->protosetup_to_you = NULL;

    iceConn->connect_to_me = NULL;
    iceConn->protosetup_to_me = NULL;


    /*
     * Send our byte order.
     */

    IceGetHeader (iceConn, 0, ICE_ByteOrder,
	SIZEOF (iceByteOrderMsg), iceByteOrderMsg, pByteOrderMsg);

    endian = 1;
    if (*(char *) &endian)
	pByteOrderMsg->byteOrder = IceLSBfirst;
    else
	pByteOrderMsg->byteOrder = IceMSBfirst;

    IceFlush (iceConn);


    /*
     * Now read the ByteOrder message from the other client.
     * iceConn->swap should be set to the appropriate boolean
     * value after the call to IceProcessMessages.
     */

    iceConn->waiting_for_byteorder = True;

    while (iceConn->waiting_for_byteorder == True)
	IceProcessMessages (iceConn, NULL);

    if (iceConn->connection_status == IceConnectRejected)
    {
	/*
	 * We failed to get the required ByteOrder message.
	 */

	_IceFreeConnection (iceConn, True);
	strncpy (errorStringRet,
	    "Internal error - did not receive the expected ByteOrder message",
	     errorLength);
	return (NULL);
    }


    /*
     * Determine which authentication methods are available for
     * the Connection Setup authentication.
     */

    _IceGetPoValidAuthIndices (
	"ICE", iceConn->connection_string,
	_IceAuthCount, _IceAuthNames, &authUsableCount, authIndices);

    for (i = 0; i < _IceAuthCount; i++)
    {
	authUsableFlags[i] = 0;
	for (j = 0; j < authUsableCount && !authUsableFlags[i]; j++)
	    authUsableFlags[i] = (authIndices[j] == i);
    }


    /*
     * Now send a Connection Setup message.
     */

    extra = STRING_BYTES (IceVendorString) + STRING_BYTES (IceReleaseString);

    for (i = 0; i < _IceAuthCount; i++)
	if (authUsableFlags[i])
	{
	    extra += STRING_BYTES (_IceAuthNames[i]);
	}

    extra += (_IceVersionCount * 4);

    IceGetHeaderExtra (iceConn, 0, ICE_ConnectionSetup,
	SIZEOF (iceConnectionSetupMsg), WORD64COUNT (extra),
	iceConnectionSetupMsg, pSetupMsg, pData);

    setup_sequence = iceConn->send_sequence;

    pSetupMsg->versionCount = _IceVersionCount;
    pSetupMsg->authCount = authUsableCount;
    pSetupMsg->mustAuthenticate = mustAuthenticate;

    STORE_STRING (pData, IceVendorString);
    STORE_STRING (pData, IceReleaseString);

    for (i = 0; i < _IceAuthCount; i++)
	if (authUsableFlags[i])
	{
	    STORE_STRING (pData, _IceAuthNames[i]);
	}

    for (i = 0; i < _IceVersionCount; i++)
    {
	STORE_CARD16 (pData, _IceVersions[i].major_version);
	STORE_CARD16 (pData, _IceVersions[i].minor_version);
    }

    IceFlush (iceConn);


    /*
     * Process messages until we get a Connection Reply or an Error Message.
     * Authentication will take place behind the scenes.
     */

    replyWait.sequence_of_request = setup_sequence;
    replyWait.major_opcode_of_request = 0;
    replyWait.minor_opcode_of_request = ICE_ConnectionSetup;
    replyWait.reply = (IcePointer) &reply;

    gotReply = False;

    while (gotReply == False)
	if ((gotReply = IceProcessMessages (iceConn, &replyWait)) == True)
	{
	    if (reply.type == ICE_CONNECTION_REPLY)
	    {
		if (reply.connection_reply.version_index >= _IceVersionCount)
		{
		    strncpy (errorStringRet,
	    		"Got a bad version index in the Connection Reply",
			errorLength);

		    free (reply.connection_reply.vendor);
		    free (reply.connection_reply.release);
		    _IceFreeConnection (iceConn, True);
		    iceConn = NULL;
		}
		else
		{
		    iceConn->my_ice_version_index =
			reply.connection_reply.version_index;
		    iceConn->vendor = reply.connection_reply.vendor;
		    iceConn->release = reply.connection_reply.release;

		    _IceConnectionObjs[_IceConnectionCount] = iceConn;
		    _IceConnectionStrings[_IceConnectionCount] =
			iceConn->connection_string;
		    _IceConnectionCount++;

		    free ((char *) iceConn->connect_to_you);
		    iceConn->connect_to_you = NULL;

		    iceConn->connection_status = IceConnectAccepted;
		}
	    }
	    else /* reply.type == ICE_CONNECTION_ERROR */
	    {
		/* Connection failed */

		strncpy (errorStringRet, reply.connection_error.error_message,
		    errorLength);

		free (reply.connection_error.error_message);

		_IceFreeConnection (iceConn, True);
		iceConn = NULL;
	    }
	}

    if (iceConn && _IceWatchProcs)
    {
	/*
	 * Notify the watch procedures that an iceConn was opened.
	 */

	_IceConnectionOpened (iceConn);
    }

    return (iceConn);
}



/* ------------------------------------------------------------------------- *
 *                            local routines                                 *
 * ------------------------------------------------------------------------- */

#define ICE_CONNECTION_RETRIES 5


static XtransConnInfo
ConnectToPeer (networkIdsList, actualConnectionRet)

char *networkIdsList;
char **actualConnectionRet;

{
    char address[256];
    char *ptr, *endptr, *delim;
    int  madeConnection = 0;
    int  len, retry;
    int  connect_stat;
    XtransConnInfo trans_conn = NULL;

    *actualConnectionRet = NULL;

    ptr = networkIdsList;
    endptr = networkIdsList + strlen (networkIdsList);

    while (ptr < endptr && !madeConnection)
    {
	if ((delim = (char *) strchr (ptr, ',')) == NULL)
	    delim = endptr;

	len = delim - ptr;
	strncpy (address, ptr, len);
	address[len] = '\0';

	ptr = delim + 1;

	for (retry = ICE_CONNECTION_RETRIES; retry >= 0; retry--)
	{
	    if ((trans_conn = _IceTransOpenCOTSClient (address)) == NULL)
	    {
		break;
	    }

	    if ((connect_stat = _IceTransConnect (trans_conn, address)) < 0)
	    {
		_IceTransClose (trans_conn);

		if (connect_stat == TRANS_TRY_CONNECT_AGAIN)
		{
		    sleep(1);
		    continue;
		}
		else
		    break;
	    }
	    else
	    {
		madeConnection = 1;
		break;
	    }
	}
    }


    if (madeConnection)
    {
	/*
	 * We need to return the actual network connection string
	 */

	*actualConnectionRet = (char *) malloc (strlen (address) + 1);
	strcpy (*actualConnectionRet, address);

	
	/*
	 * Return the file descriptor
	 */

	return (trans_conn);
    }
    else
    {
	return (NULL);
    }
}
