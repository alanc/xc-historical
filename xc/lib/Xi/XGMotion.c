/* $XConsortium: XGMotion.c,v 1.6 90/05/18 11:23:19 rws Exp $ */

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
 * XGetDeviceMotionEvents - Get the motion history of an input device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"
#include "extutil.h"

XDeviceTimeCoord
*XGetDeviceMotionEvents (dpy, dev, start, stop, nEvents, mode, axis_count)
    register 	Display	*dpy;
    XDevice		*dev;
    Time 		start;
    Time 		stop;
    int 		*nEvents;
    int 		*mode;
    int 		*axis_count;
    {       
    char **tmp;
    xGetDeviceMotionEventsReq 	*req;
    xGetDeviceMotionEventsReply 	rep;
    XDeviceTimeCoord *tc;
    int *data;
    char *bufp, *readp, *savp;
    long size, size2;
    int	 i, j;
    XExtDisplayInfo *info = (XExtDisplayInfo *) XInput_find_display (dpy);

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XDeviceTimeCoord *) NoSuchExtension);

    GetReq(GetDeviceMotionEvents,req);		
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_GetDeviceMotionEvents;
    req->start = start;
    req->stop = stop;
    req->deviceid = dev->device_id;

    if (!_XReply (dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
        SyncHandle();
	*nEvents = 0;
	return (NULL);
	}

    *mode = rep.mode;
    *axis_count = rep.axes;
    *nEvents = rep.nEvents;
    size = rep.length << 2;
    size2 = rep.nEvents * 
	(sizeof (XDeviceTimeCoord) + (rep.axes * sizeof (int)));
    savp = readp = (char *) Xmalloc (size);
    bufp = (char *) Xmalloc (size2);
    if (!bufp || !savp)
	{
	*nEvents = 0;
	_XEatData (dpy, (unsigned long) size);
	UnlockDisplay(dpy);
	SyncHandle();
	return (NULL);
	}
    _XRead (dpy, (char *) readp, size);

    tc = (XDeviceTimeCoord *) bufp;
    data = (int *) ((char *) bufp + (rep.nEvents * sizeof (XDeviceTimeCoord)));
    for (i=0; i<*nEvents; i++)
	{
	tc->time = *((CARD16 *) readp) << 16;
	readp += sizeof (CARD16);
	tc->time |= *((CARD16 *) readp);
	readp += sizeof (CARD16);

	(tc++)->data = data;

	for (j=0; j<*axis_count; j++)
	    {
	    *data++ = *((INT16 *) readp);
	    readp += sizeof (INT16);
	    }
	}
    XFree (savp);
    UnlockDisplay(dpy);
    SyncHandle();
    return ((XDeviceTimeCoord *) bufp);
    }

