/* $XConsortium: XGtSelect.c,v 1.9 94/02/14 17:39:29 rws Exp $ */

/************************************************************
Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, California, and the 
Massachusetts Institute of Technology, Cambridge, Massachusetts.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Hewlett-Packard or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * XGetSelectedExtensionEvents - return a list of currently selected events.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"
#include "extutil.h"

int
XGetSelectedExtensionEvents (dpy, w, this_client_count, this_client_list, 
	all_clients_count, all_clients_list)
    register 	Display *dpy;
    Window 	w;
    int		*this_client_count;
    XEventClass	**this_client_list;
    int		*all_clients_count;
    XEventClass	**all_clients_list;
    {
    int		tlen, alen;
    register 	xGetSelectedExtensionEventsReq *req;
    xGetSelectedExtensionEventsReply rep;
    XExtDisplayInfo *info = (XExtDisplayInfo *) XInput_find_display (dpy);

    LockDisplay (dpy);
    if (_XiCheckExtInit(dpy, XInput_Initial_Release) == -1)
	return (NoSuchExtension);
    GetReq(GetSelectedExtensionEvents,req);		

    req->reqType = info->codes->major_opcode;
    req->ReqType = X_GetSelectedExtensionEvents;
    req->window = w;

    if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) 
	{
	UnlockDisplay(dpy);
	SyncHandle();
	return Success;
	}

    *this_client_count = rep.this_client_count;
    *all_clients_count = rep.all_clients_count;

    if (rep.length)
	{
	tlen = (*this_client_count) * sizeof(XEventClass);
	alen = (rep.length << 2) - tlen;

	if (tlen)
	    {
	    *this_client_list = (XEventClass *) Xmalloc (tlen);
	    if (!*this_client_list) 
		{
	        _XEatData (dpy, (unsigned long) tlen+alen);
		return (Success);
		}
	    _XRead (dpy, (char *)*this_client_list, tlen);
	    }
	else
	    *this_client_list = (XEventClass *) NULL;
	if (alen)
	    {
	    *all_clients_list = (XEventClass *) Xmalloc (alen);
	    if (!*all_clients_list) 
		{
		Xfree((char *)*this_client_list);
		*this_client_list = NULL;
	        _XEatData (dpy, (unsigned long) alen);
		return (Success);
		}
	    _XRead (dpy, (char *)*all_clients_list, alen);
	    }
	else
	    *all_clients_list = (XEventClass *) NULL;

	}

    UnlockDisplay(dpy);
    SyncHandle();
    return (Success);
    }
