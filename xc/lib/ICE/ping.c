/* $XConsortium: ping.c,v 1.1 93/08/17 18:58:59 mor Exp $ */
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

void
IcePing (iceConn, pingReplyCB)

IceConn		iceConn;
IcePingReplyCB	pingReplyCB;

{
    if (iceConn->ping_reply_cb == NULL)
    {
	IceSimpleMessage (iceConn, 0, ICE_Ping);
	IceFlush (iceConn);

	iceConn->ping_reply_cb = pingReplyCB;
    }
}
