/* $XConsortium: watch.c,v 1.2 93/09/14 15:36:04 mor Exp $ */
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


Status
IceAddConnectionWatch (watchProc, clientData)

IceWatchProc	watchProc;
IcePointer	clientData;

{
    /*
     * watchProc will be called each time an ICE connection is
     * created/destroyed by ICElib.
     */

    _IceWatchProc	*ptr = _IceWatchProcs;
    _IceWatchProc	*newWatchProc;

    if ((newWatchProc = (_IceWatchProc *) malloc (
	sizeof (_IceWatchProc))) == NULL)
    {
	return (0);
    }

    newWatchProc->watch_proc = watchProc;
    newWatchProc->client_data = clientData;
    newWatchProc->watched_connections = NULL;
    newWatchProc->next = NULL;

    while (ptr && ptr->next)
	ptr = ptr->next;

    if (ptr == NULL)
	_IceWatchProcs = newWatchProc;
    else
	ptr->next = newWatchProc;

    return (1);
}



void
IceRemoveConnectionWatch (watchProc, clientData)

IceWatchProc	watchProc;
IcePointer	clientData;

{
    _IceWatchProc	*currWatchProc = _IceWatchProcs;
    _IceWatchProc	*prevWatchProc = NULL;

    while (currWatchProc && (currWatchProc->watch_proc != watchProc ||
        currWatchProc->client_data != clientData))
    {
	prevWatchProc = currWatchProc;
	currWatchProc = currWatchProc->next;
    }

    if (currWatchProc)
    {
	_IceWatchProc		*nextWatchProc = currWatchProc->next;
	_IceWatchedConnection 	*watchedConn;

	watchedConn = currWatchProc->watched_connections;
	while (watchedConn)
	{
	    _IceWatchedConnection *nextWatchedConn = watchedConn->next;
	    free ((char *) watchedConn);
	    watchedConn = nextWatchedConn;
	}

	if (prevWatchProc == NULL)
	    _IceWatchProcs = nextWatchProc;
	else
	    prevWatchProc->next = nextWatchProc;

	free ((char *) currWatchProc);
    }
}



void
_IceConnectionOpened (iceConn)

IceConn	iceConn;

{
    _IceWatchProc *watchProc = _IceWatchProcs;

    while (watchProc)
    {
	_IceWatchedConnection *newWatchedConn = (_IceWatchedConnection *)
	    malloc (sizeof (_IceWatchedConnection));
	_IceWatchedConnection *watchedConn;

	watchedConn = watchProc->watched_connections;
	while (watchedConn && watchedConn->next)
	    watchedConn = watchedConn->next;

	newWatchedConn->iceConn = iceConn;
	newWatchedConn->next = NULL;

	if (watchedConn == NULL)
	    watchProc->watched_connections = newWatchedConn;
	else
	    watchedConn->next = newWatchedConn;

	(*watchProc->watch_proc) (iceConn,
	    watchProc->client_data, True, &newWatchedConn->watch_data);

	watchProc = watchProc->next;
    }
}



void
_IceConnectionClosed (iceConn)

IceConn	iceConn;

{
    _IceWatchProc *watchProc = _IceWatchProcs;

    while (watchProc)
    {
	_IceWatchedConnection *watchedConn = watchProc->watched_connections;
	_IceWatchedConnection *prev = NULL;

	while (watchedConn && watchedConn->iceConn != iceConn)
	{
	    prev = watchedConn;
	    watchedConn = watchedConn->next;
	}

	if (watchedConn)
	{
	    (*watchProc->watch_proc) (iceConn,
	        watchProc->client_data, False, &watchedConn->watch_data);

	    if (prev == NULL)
		watchProc->watched_connections = watchedConn->next;
	    else
		prev->next = watchedConn->next;

	    free ((char *) watchedConn);
	}

	watchProc = watchProc->next;
    }
}
