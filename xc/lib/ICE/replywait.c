/* $XConsortium: replywait.c,v 1.1 93/08/26 17:34:41 mor Exp $ */
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


void
_IceAddReplyWait (iceConn, replyWait)

IceConn			iceConn;
IceReplyWaitInfo	*replyWait;

{
    /*
     * Add this replyWait to the end of the list (only if the
     * replyWait is not already in the list).
     */

    _IceSavedReplyWait	*savedReplyWait;
    _IceSavedReplyWait	*prev, *last;

    prev = NULL;
    last = iceConn->saved_reply_waits;

    while (last)
    {
	if (last->reply_wait == replyWait)
	    return;

	prev = last;
	last = last->next;
    }
	
    savedReplyWait = (_IceSavedReplyWait *) malloc (
	sizeof (_IceSavedReplyWait));

    savedReplyWait->reply_wait = replyWait;
    savedReplyWait->reply_ready = False;
    savedReplyWait->next = NULL;

    if (prev == NULL)
	iceConn->saved_reply_waits = savedReplyWait;
    else
	prev->next = savedReplyWait;
}



IceReplyWaitInfo *
_IceSearchReplyWaits (iceConn, majorOpcode)

IceConn	iceConn;
int	majorOpcode;

{
    /*
     * Return the first replyWait in the list with the given majorOpcode
     */

    _IceSavedReplyWait	*savedReplyWait = iceConn->saved_reply_waits;

    while (savedReplyWait && !savedReplyWait->reply_ready &&
	savedReplyWait->reply_wait->major_opcode_of_request != majorOpcode)
    {
	savedReplyWait = savedReplyWait->next;
    }

    return (savedReplyWait ? savedReplyWait->reply_wait : NULL);
}



void
_IceSetReplyReady (iceConn, replyWait)

IceConn			iceConn;
IceReplyWaitInfo	*replyWait;

{
    /*
     * The replyWait specified has a reply ready.
     */

    _IceSavedReplyWait	*savedReplyWait = iceConn->saved_reply_waits;

    while (savedReplyWait && savedReplyWait->reply_wait != replyWait)
	savedReplyWait = savedReplyWait->next;

    if (savedReplyWait)
	savedReplyWait->reply_ready = True;
}



Bool
_IceCheckReplyReady (iceConn, replyWait)

IceConn			iceConn;
IceReplyWaitInfo	*replyWait;

{
    _IceSavedReplyWait	*savedReplyWait = iceConn->saved_reply_waits;
    _IceSavedReplyWait	*prev = NULL;
    Bool		found = False;
    Bool		ready;

    while (savedReplyWait && !found)
    {
	if (savedReplyWait->reply_wait == replyWait)
	    found = True;
	else
	{
	    prev = savedReplyWait;
	    savedReplyWait = savedReplyWait->next;
	}
    }

    ready = found && savedReplyWait->reply_ready;

    if (ready)
    {
	if (prev == NULL)
	    iceConn->saved_reply_waits = savedReplyWait->next;
	else
	    prev->next = savedReplyWait->next;
	
	free ((char *) savedReplyWait);
    }

    return (ready);
}
