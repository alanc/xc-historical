/* $XConsortium: XStFocus.c,v 1.3 89/09/25 16:21:23 gms Exp $ */

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
 * XSetDeviceFocus - Set the focus of an extension device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"

extern int	IReqCode;

int
XSetDeviceFocus(dpy, dev, focus, revert_to, time)
    register 	Display *dpy;
    XDevice 	*dev;
    Window 	focus;
    int 	revert_to;
    Time	time;
    {       
    xSetDeviceFocusReq 	*req;

    LockDisplay (dpy);

    GetReq(SetDeviceFocus,req);		
    req->reqType = IReqCode;
    req->ReqType = X_SetDeviceFocus;
    req->device = dev->device_id;
    req->focus = focus;
    req->revertTo = revert_to;
    req->time = time;

    UnlockDisplay(dpy);
    SyncHandle();
    return (Success);
    }
