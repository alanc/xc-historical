/* $XConsortium: connect.c,v 1.2 93/08/19 10:23:39 mor Exp $ */
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
#include "globals.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef TCPCONN
# include <netinet/in.h>
# ifndef hpux
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   include <netinet/tcp.h>
#  endif
# endif
#endif

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif

#ifdef UNIXCONN
#include <sys/un.h>
#endif

#define ICE_CONNECTION_RETRIES 5

static int  ConnectToPeer();

IceConn     	_IceConnectionObjs[256];
char	    	*_IceConnectionStrings[256];
static int     	_IceConnectionCount = 0;



IceConn
IceOpenConnection (networkIdsList, errorLength, errorStringRet)

char *networkIdsList;
int  errorLength;
char *errorStringRet;

{
    IceConn			iceConn;
    int				extra, i;
    int		       		endian;
    Bool			gotReply;
    unsigned long		setup_sequence;
    iceByteOrderMsg		*pByteOrderMsg;
    iceConnectionSetupMsg	*pSetupMsg;
    char			*pData;
    IceReplyWaitInfo 		replyWait;
    _IceReply		 	reply;

    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    if (networkIdsList == NULL || *networkIdsList == '\0')
    {
	strncpy (errorStringRet,
	    "networkIdsList argument is NULL", errorLength);
	return (NULL);
    }

    for (i = 0; i < _IceConnectionCount; i++)
    {
	char *strptr;
	if ((strptr = (char *) strstr (
	    networkIdsList, _IceConnectionStrings[i])) != NULL)
	{
	    char ch = *(strptr + strlen (_IceConnectionStrings[i]));
	    if (ch == ',' || ch == '\0')
	    {
		_IceConnectionObjs[i]->ref_count++;
		return (_IceConnectionObjs[i]);
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

    if ((iceConn->fd = ConnectToPeer (networkIdsList,
	&iceConn->connection_string)) < 0)
    {
	_IceFreeConnection (iceConn);
	strncpy (errorStringRet, "Could not open network socket", errorLength);
	return (NULL);
    }

    iceConn->connection_status = IceConnectPending;
    iceConn->my_ice_version_index = 0;
    iceConn->ref_count = 1;
    iceConn->sequence = 0;
    iceConn->ping_reply_cb = NULL;

    if ((iceConn->inbuf = iceConn->inbufptr =
	(char *) malloc (ICE_INBUFSIZE)) == NULL)
    {
	_IceFreeConnection (iceConn);
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }

    iceConn->inbufmax = iceConn->inbuf + ICE_INBUFSIZE;

    if ((iceConn->outbuf = iceConn->outbufptr =
	(char *) malloc (ICE_OUTBUFSIZE)) == NULL)
    {
	_IceFreeConnection (iceConn);
	strncpy (errorStringRet, "Can't malloc", errorLength);
	return (NULL);
    }

    iceConn->outbufmax = iceConn->outbuf + ICE_OUTBUFSIZE;

    iceConn->scratch = NULL;
    iceConn->scratch_size = 0;

    iceConn->process_msg_info = NULL;

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
	pByteOrderMsg->byteOrder = IceLittleEndian;
    else
	pByteOrderMsg->byteOrder = IceBigEndian;

    IceFlush (iceConn);


    /*
     * Now read the ByteOrder message from the other client.
     * iceConn->swap should be set to the appropriate boolean
     * value after the call to IceProcessMessage.
     */

    iceConn->waiting_for_byteorder = 1;

    IceProcessMessage (iceConn, NULL);

    if (iceConn->connection_status == IceConnectRejected)
    {
	/*
	 * We failed to get the required ByteOrder message.
	 */

	_IceFreeConnection (iceConn);
	strncpy (errorStringRet,
	    "Internal error - did not receive the expected ByteOrder message",
	     errorLength);
	return (NULL);
    }


    /*
     * Now send a Connection Setup message.
     */

    extra = XPCS_BYTES (IceVendorString) + XPCS_BYTES (IceReleaseString);
    for (i = 0; i < _IceAuthCount; i++)
	extra += XPCS_BYTES (_IceOCLauthRecs[i].auth_name);
    extra += (_IceVersionCount * 2);

    IceGetHeaderExtra (iceConn, 0, ICE_ConnectionSetup,
	SIZEOF (iceConnectionSetupMsg), WORD64COUNT (extra),
	iceConnectionSetupMsg, pSetupMsg, pData);

    setup_sequence = iceConn->sequence;

    pSetupMsg->versionCount = _IceVersionCount;
    pSetupMsg->authCount    = _IceAuthCount;

    STORE_XPCS (pData, IceVendorString);
    STORE_XPCS (pData, IceReleaseString);

    for (i = 0; i < _IceAuthCount; i++)
    {
	STORE_XPCS (pData, _IceOCLauthRecs[i].auth_name);
    }

    for (i = 0; i < _IceVersionCount; i++)
    {
	STORE_CARD8 (pData, _IceVersions[i].major_version);
	STORE_CARD8 (pData, _IceVersions[i].minor_version);
    }

    IceFlush (iceConn);


    /*
     * Process messages until we get a Connection Reply or an Error Message.
     * Authentication will take place behind the scenes.
     */

    replyWait.opcode_of_request = ICE_ConnectionSetup;
    replyWait.sequence_of_request = setup_sequence;
    replyWait.reply = (IcePointer) &reply;

    gotReply = False;

    while (gotReply == False)
	if ((gotReply = IceProcessMessage (iceConn, &replyWait)) == True)
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
		    _IceFreeConnection (iceConn);
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

		_IceFreeConnection (iceConn);
		iceConn = NULL;
	    }
	}

    return (iceConn);
}



void
IceCloseConnection (iceConn)

IceConn     iceConn;

{
    int i;

    for (i = 0; i < _IceConnectionCount; i++)
	if (_IceConnectionObjs[i] == iceConn)
	    break;

    if (i < _IceConnectionCount && (--iceConn->ref_count == 0))
    {
	_IceFreeConnection (iceConn);

	if (i < _IceConnectionCount - 1)
	{
	    _IceConnectionObjs[i] =
		_IceConnectionObjs[_IceConnectionCount - 1];
	    _IceConnectionStrings[i] =
	        _IceConnectionStrings[_IceConnectionCount - 1];
	}

	_IceConnectionCount--;
    }
}



/* ------------------------------------------------------------------------- *
 *                            local routines                                 *
 * ------------------------------------------------------------------------- */

static int
ConnectToPeer (networkIdsList, actualConnectionRet)

char *networkIdsList;
char **actualConnectionRet;

{
    char hostname[256];
    char portnum[20];
    char objname[20];
    char pathname[256];
    char *ptr, *delim;
    char *saveptr, *endptr;
    int  madeConnection = 0;
    char *peer_addr = NULL;
    int  peer_addrlen = 0;
    int  fd = -1;

    *actualConnectionRet = NULL;

    ptr = networkIdsList;
    endptr = networkIdsList + strlen (networkIdsList);

    while (ptr && ptr < endptr && !madeConnection)
    {
	saveptr = ptr;

	if (strncmp (ptr, "local/", 6) == 0)
	{
	    ptr += 6;
	    if ((delim = (char *) strchr (ptr, ':')) != NULL)
	    {
		strncpy (hostname, ptr, delim - ptr);
		hostname[delim - ptr] = '\0';
		ptr = delim + 1;
		if ((delim = (char *) strchr (ptr, ',')) == 0)
		    delim = (char *) strchr (ptr, '\0');

		strncpy (pathname, ptr, delim - ptr);
		pathname[delim - ptr] = '\0';
		ptr = delim + 1;
	    }
	    else if ((ptr = (char *) strchr (ptr, ',')) != NULL)
		ptr++;
#ifdef UNIXCONN
	    fd = _MakeConnection ("Unix", 0, pathname,
	        ICE_CONNECTION_RETRIES, &peer_addrlen, &peer_addr);

	    if (fd >= 0 && fd < OPEN_MAX)
	    {
		madeConnection = 1;

#if 0
		peer_addrlen = _GetHostname (tmpbuf, sizeof (tmpbuf));
		peer_addr = malloc (peer_addrlen + 1);
		strcpy (peer_addr, tmpbuf);
#endif

	    }
#endif
	}
	else if (strncmp (ptr, "tcp/", 4) == 0)
	{
	    ptr += 4;
	    if ((delim = (char *) strchr (ptr, ':')) != NULL)
	    {
		strncpy (hostname, ptr, delim - ptr);
		hostname[delim - ptr] = '\0';
		ptr = delim + 1;
		if ((delim = (char *) strchr (ptr, ',')) == 0)
		    delim = (char *) strchr (ptr, '\0');

		strncpy (portnum, ptr, delim - ptr);
		portnum[delim - ptr] = '\0';
		ptr = delim + 1;
	    }
	    else if ((ptr = (char *) strchr (ptr, ',')) != NULL)
		ptr++;

#ifdef TCPCONN
	    fd = _MakeConnection ("TCP", hostname, portnum,
	        ICE_CONNECTION_RETRIES, &peer_addrlen, &peer_addr);

	    if (fd >= 0 && fd < OPEN_MAX)
		madeConnection = 1;
#endif
	}
	else if (strncmp (ptr, "decnet/", 7) == 0)
	{
	    ptr += 7;
	    if ((delim = (char *) strchr (ptr, ':')) != NULL)
	    {
		delim++;  /* skip 2nd : */
		strncpy (hostname, ptr, delim - ptr);
		hostname[delim - ptr] = '\0';
		ptr = delim + 1;
		if ((delim = (char *) strchr (ptr, ',')) == 0)
		    delim = (char *) strchr (ptr, '\0');

		strncpy (objname, ptr, delim - ptr);
		objname[delim - ptr] = '\0';
		ptr = delim + 1;
	    }
	    else if ((ptr = (char *) strchr (ptr, ',')) != NULL)
		ptr++;

#ifdef DNETCONN

	    fd = _MakeConnection ("DECnet", peerbuf, objname,
		ICE_CONNECTION_RETRIES, &peer_addrlen, &peer_addr);

	    if (fd >= 0 && fd < OPEN_MAX)
		madeConnection = 1;
#endif
	}
	else if ((ptr = (char *) strchr (ptr, ',')) != NULL)
	    ptr++;
    }


    if (!madeConnection)
    {
	if (fd >= 0)
	    (void) close (fd);
	return (-1);
    }


    /*
     * We need to return the actual network connection string
     */

    *actualConnectionRet = (char *) malloc (ptr - saveptr);
    strncpy (*actualConnectionRet, saveptr, ptr - saveptr);
    (*actualConnectionRet)[ptr - saveptr - 1] = '\0';

    /*
     * Return the file descriptor
     */

    return (fd);
}
