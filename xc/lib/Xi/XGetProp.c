/* $XConsortium: XGetProp.c,v 1.3 89/09/25 16:20:34 gms Exp $ */

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
 * XGetDeviceDontPropagateList - Get the dont_propagate_list for a
 * window.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"

extern int	IReqCode;

XEventClass
*XGetDeviceDontPropagateList (dpy, window, count)
    register Display 	*dpy;
    Window 		window;
    int 		*count;
    {       
    XEventClass		*list;
    int			rlen;
    xGetDeviceDontPropagateListReq 	*req;
    xGetDeviceDontPropagateListReply 	rep;

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XEventClass *) NoSuchExtension);

    GetReq(GetDeviceDontPropagateList,req);		
    req->reqType = IReqCode;
    req->ReqType = X_GetDeviceDontPropagateList;
    req->window = window;

    (void) _XReply (dpy, (xReply *) &rep, 0, xFalse);
    *count = rep.count;

    rlen = rep.length << 2;
    list = (XEventClass *) Xmalloc (rlen);

    _XRead (dpy, list, rlen);

    UnlockDisplay(dpy);
    SyncHandle();
    return (list);
    }

