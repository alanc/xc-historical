/* $XConsortium: XOpenDev.c,v 1.3 89/09/25 16:20:58 gms Exp $ */

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
 * XOpenDevice - Request the server to open and extension input device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "XInput.h"
#include "Xlibint.h"

extern	int	IReqCode;

XDevice
*XOpenDevice(dpy, id)
    register Display 	*dpy;
    register XID	id;
    {	
    int			i;
    register long	rlen;
    xOpenDeviceReq 	*req;
    xOpenDeviceReply rep;
    XDevice *dev;

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XDevice *) NoSuchExtension);

    GetReq(OpenDevice,req);		
    req->reqType = IReqCode;
    req->ReqType = X_OpenDevice;
    req->deviceid = id;

    _XReply (dpy, (xReply *) &rep, 0, xFalse);

    rlen = rep.length << 2;
    dev = (XDevice *) Xmalloc (sizeof(XDevice) + rep.num_classes * 
	sizeof (XInputClassInfo));
    dev->device_id = req->deviceid;
    dev->num_classes = rep.num_classes;
    dev->classes = (XInputClassInfo *) ((char *) dev + sizeof (XDevice));
    _XRead (dpy, dev->classes, rlen);

    UnlockDisplay (dpy);
    SyncHandle();
    return (dev);
    }

