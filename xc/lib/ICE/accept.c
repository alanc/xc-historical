/* $XConsortium: accept.c,v 1.15 93/11/25 14:24:45 mor Exp $ */
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


IceConn
IceAcceptConnection (listenObj)

IceListenObj listenObj;

{
    IceConn    		iceConn;
    int        		newconn;
    iceByteOrderMsg 	*pMsg;
    int   		endian;

    /*
     * Accept the connection.
     */

    if ((newconn = accept (listenObj->fd,
	(struct sockaddr *) NULL, (int *) NULL)) < 0)
    {
	return (NULL);
    }


    /*
     * Create an ICE object for this connection.
     */

    if ((iceConn = (IceConn) malloc (sizeof (struct _IceConn))) == NULL)
    {
	close (newconn);
	return (NULL);
    }

    iceConn->listen_obj = listenObj;

    iceConn->waiting_for_byteorder = True;
    iceConn->connection_status = IceConnectPending;
    iceConn->my_ice_version_index = 0;

    iceConn->fd = newconn;
    iceConn->sequence = 0;

    iceConn->connection_string = (char *) malloc (
	strlen (listenObj->network_id) + 1);

    if (iceConn->connection_string == NULL)
    {
	close (newconn);
	free ((char *) iceConn);
	return (NULL);
    }
    else
	strcpy (iceConn->connection_string, listenObj->network_id);

    iceConn->vendor = NULL;
    iceConn->release = NULL;

    if ((iceConn->inbuf = iceConn->inbufptr =
	(char *) malloc (ICE_INBUFSIZE)) != NULL)
    {
	iceConn->inbufmax = iceConn->inbuf + ICE_INBUFSIZE;
    }
    else
    {
	close (newconn);
	free ((char *) iceConn);
	return (NULL);
    }

    if ((iceConn->outbuf = iceConn->outbufptr =
	(char *) malloc (ICE_OUTBUFSIZE)) != NULL)
    {
	iceConn->outbufmax = iceConn->outbuf + ICE_OUTBUFSIZE;
    }
    else
    {
	close (newconn);
	free (iceConn->inbuf);
	free ((char *) iceConn);
	return (NULL);
    }

    iceConn->scratch = NULL;
    iceConn->scratch_size = 0;

    iceConn->open_ref_count = 1;
    iceConn->proto_ref_count = 0;

    iceConn->skip_want_to_close = False;
    iceConn->want_to_close = False;

    iceConn->saved_reply_waits = NULL;
    iceConn->ping_waits = NULL;

    iceConn->process_msg_info = NULL;

    iceConn->connect_to_you = NULL;
    iceConn->protosetup_to_you = NULL;

    iceConn->connect_to_me = NULL;
    iceConn->protosetup_to_me = NULL;


    /*
     * Send our byte order.
     */

    IceGetHeader (iceConn, 0, ICE_ByteOrder,
	SIZEOF (iceByteOrderMsg), iceByteOrderMsg, pMsg);

    endian = 1;
    if (*(char *) &endian)
	pMsg->byteOrder = IceLittleEndian;
    else
	pMsg->byteOrder = IceBigEndian;

    IceFlush (iceConn);


    if (_IceWatchProcs)
    {
	/*
	 * Notify the watch procedures that an iceConn was opened.
	 */

	_IceConnectionOpened (iceConn);
    }

    return (iceConn);
}
