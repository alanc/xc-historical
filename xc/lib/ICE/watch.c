/* $XConsortium: connect.c,v 1.6 93/09/08 20:02:20 mor Exp $ */
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
IceAddConnectionWatch (watchProc, clientData)

IceWatchProc	watchProc;
IcePointer	clientData;

{
    /*
     * watchProc will be called each time an ICE connection is
     * created/destroyed by ICElib.
     */

    _IceWatchProc *watch = (_IceWatchProc *) malloc (sizeof (_IceWatchProc));
    _IceWatchProc *ptr = _IceWatchProcs;

    if (watch == NULL)
	return (0);

    watch->watch_proc = watchProc;
    watch->client_data = clientData;
    watch->next = NULL;

    while (ptr && ptr->next)
	ptr = ptr->next;

    if (ptr == NULL)
	_IceWatchProcs = watch;
    else
	ptr->next = watch;

    return (1);
}



void
IceRemoveConnectionWatch (watchProc, clientData)

IceWatchProc	watchProc;
IcePointer	clientData;

{
    _IceWatchProc *watch = _IceWatchProcs;
    _IceWatchProc *prev = NULL;

    while (watch && (watch->watch_proc != watchProc ||
        watch->client_data != clientData))
    {
	prev = watch;
	watch = watch->next;
    }

    if (watch)
    {
	_IceWatchProc *next = watch->next;

	if (prev == NULL)
	    _IceWatchProcs = next;
	else
	    prev->next = next;

	free ((char *) watch);
    }
}
