/* $XConsortium: xtwatch.c,v 1.4 94/03/21 13:28:21 mor Exp $ */
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
#include <X11/Intrinsic.h>


Status
InitWatchProcs (appContext)

XtAppContext appContext;

{
    void _XtIceWatchProc ();

    return (IceAddConnectionWatch (_XtIceWatchProc, (IcePointer) appContext));
}


void
_XtIceWatchProc (ice_conn, client_data, opening, watch_data)

IceConn 	ice_conn;
IcePointer	client_data;
Bool		opening;
IcePointer	*watch_data;

{
    if (opening)
    {
	XtAppContext appContext = (XtAppContext) client_data;
	void _XtProcessIceMsgProc ();

	*watch_data = (IcePointer) XtAppAddInput (
	    appContext,
	    IceConnectionNumber (ice_conn),
            (XtPointer) XtInputReadMask,
	    _XtProcessIceMsgProc,
	    (XtPointer) ice_conn);
    }
    else
    {
	XtRemoveInput ((XtInputId) *watch_data);
    }
}


void
_XtProcessIceMsgProc (client_data, source, id)

XtPointer	client_data;
int 		*source;
XtInputId	*id;

{
    IceConn	ice_conn = (IceConn) client_data;

    IceProcessMessages (ice_conn, NULL);
}
