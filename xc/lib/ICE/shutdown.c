/* $XConsortium: shutdown.c,v 1.2 93/09/12 14:21:29 mor Exp $ */
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
IceProtocolShutdown (iceConn, majorOpcode)

IceConn iceConn;
int	majorOpcode;

{
    if (majorOpcode < 1 || majorOpcode > _IceLastMajorOpcode ||
        iceConn->proto_ref_count == 0)
    {
	return (0);
    }
    else
    {
	/*
	 * TO DO:
	 * We should really check that the majorOpcode is being used.
	 * If it's not, we should return bad status.
	 */

	iceConn->proto_ref_count--;

	return (1);
    }
}



void
IceSkipShutdownNegotiation (iceConn)

IceConn     iceConn;

{
    iceConn->skip_want_to_close = True;
}



Bool
IceCheckShutdownNegotiation (iceConn)

IceConn     iceConn;

{
    return (iceConn->skip_want_to_close ? False : True);
}



void
IceCloseConnection (iceConn)

IceConn     iceConn;

{
    /*
     * If this connection object was never valid, we can close
     * it right now.  This happens if IceAcceptConnection was
     * called, but after calling IceProcessMessage several times
     * the connection was rejected (because of authentication or
     * some other reason).
     */

    if (iceConn->iceConn_type == 2 &&
	iceConn->connection_status != IceConnectAccepted)
    {
	_IceFreeConnection (iceConn, False);
	return;
    }


    /*
     * Now we try to close the connection only if all calls to
     * IceOpenConnection or IceAcceptConnection have been matched
     * with a call to IceCloseConnection, AND there are no active
     * protocols.
     */

    if (iceConn->open_ref_count > 0)
	iceConn->open_ref_count--;

    if (iceConn->open_ref_count == 0 && iceConn->proto_ref_count == 0)
    {
	if (iceConn->skip_want_to_close)
	{
	    _IceFreeConnection (iceConn, False);
	}
	else
	{
	    IceSimpleMessage (iceConn, 0, ICE_WantToClose);
	    IceFlush (iceConn);

	    iceConn->want_to_close = 1;
	}
    }
}



void
_IceFreeConnection (iceConn, ignoreWatchProcs)

IceConn iceConn;
Bool	ignoreWatchProcs;

{
    if (!ignoreWatchProcs)
    {
	_IceWatchProc *watchProc = _IceWatchProcs;

	while (watchProc)
	{
	    (*watchProc->watch_proc) (iceConn,
		watchProc->client_data, False, &iceConn->watch_data);
	    watchProc = watchProc->next;
	}
    }

    if (iceConn->iceConn_type == 1)
    {
	/*
	 * This iceConn was created with IceOpenConnection.
	 * We keep track of all open IceConn's, so we need
	 * to remove it from the list.
	 */

	int i;

	for (i = 0; i < _IceConnectionCount; i++)
	    if (_IceConnectionObjs[i] == iceConn)
		break;

	if (i < _IceConnectionCount)
	{
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

    if (iceConn->fd >= 0)
	close (iceConn->fd);

    if (iceConn->connection_string)
	free (iceConn->connection_string);

    if (iceConn->vendor)
	free (iceConn->vendor);

    if (iceConn->release)
	free (iceConn->release);

    if (iceConn->inbuf)
	free (iceConn->inbuf);

    if (iceConn->outbuf)
	free (iceConn->outbuf);

    if (iceConn->scratch)
	free (iceConn->scratch);

    if (iceConn->process_msg_info)
	free ((char *) iceConn->process_msg_info);

    if (iceConn->connect_to_you)
	free ((char *) iceConn->connect_to_you);

    if (iceConn->protosetup_to_you)
	free ((char *) iceConn->protosetup_to_you);

    if (iceConn->connect_to_me)
	free ((char *) iceConn->connect_to_me);

    if (iceConn->protosetup_to_me)
	free ((char *) iceConn->protosetup_to_me);

    free ((char *) iceConn);
}




