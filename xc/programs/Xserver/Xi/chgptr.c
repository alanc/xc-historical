#ifdef XINPUT
/* $XConsortium: xchgptr.c,v 1.4 89/10/10 16:08:37 gms Exp $ */

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
 * Extension function to change the pointer device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"

extern	int 		IReqCode;
extern	int 		BadDevice;
extern	int 		ChangeDeviceNotify;
extern	InputInfo	inputInfo;
extern	void		(* ReplySwapVector[256]) ();
DeviceIntPtr		LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure is invoked to swap the request bytes if the server and
 * client have a different byte order.
 *
 */

int
SProcXChangePointerDevice(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xChangePointerDeviceReq);
    swaps(&stuff->length, n);
    return(ProcXChangePointerDevice(client));
    }

/***********************************************************************
 *
 * This procedure changes the device used as the X pointer.
 *
 */

ProcXChangePointerDevice (client)
    register ClientPtr client;
    {
    extern		ChangePointerDevice();
    int			i;
    DeviceIntPtr 	dev;
    ValuatorClassPtr 	v;
    xChangePointerDeviceReply	rep;
    changeDeviceNotify	ev;

    REQUEST(xChangePointerDeviceReq);
    REQUEST_SIZE_MATCH(xChangePointerDeviceReq);

    rep.repType = X_Reply;
    rep.RepType = X_ChangePointerDevice;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangePointerDevice, 0, 
	    BadDevice);
	return Success;
	}

    v = dev->valuator;
    if (v == NULL || v->numAxes < 2 ||
	stuff->xaxis < 0 ||  stuff->xaxis > v->numAxes ||
	stuff->yaxis < 0 ||  stuff->yaxis > v->numAxes)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangePointerDevice, 0, BadMatch);
	return Success;
	}

    if ((dev->grab) && !SameClient(dev->grab, client))
	rep.status = AlreadyGrabbed;
    else if (dev->sync.frozen &&
	     ((dev->sync.other &&
	       !SameClient(dev->sync.other, client)) ||
	     ((dev->sync.state >= FROZEN) &&
	      !SameClient(dev->grab, client))))
	rep.status = GrabFrozen;

    if (inputInfo.pointer->focus == NULL)
	InitFocusClassDeviceStruct (inputInfo.pointer);
    ChangePointerDevice (inputInfo.pointer, dev);
    inputInfo.pointer = dev;
    inputInfo.pointer->focus->win = NULL;

    ev.type = ChangeDeviceNotify;
    ev.deviceid = stuff->deviceid;
    ev.time = currentTime.milliseconds;
    ev.request = NewPointer;

    /* 0 is the server client */
    for (i=1; i<currentMaxClients; i++)
        if (clients[i] && ! clients[i]->clientGone)
	    {
	    ev.sequenceNumber = clients[i]->sequence;
            WriteEventsToClient(clients[i], 1, &ev);
	    }

    SendMappingNotify (MappingPointer, 0, 0);

    rep.status = 0;
    WriteReplyToClient (client, sizeof (xChangePointerDeviceReply), 
	&rep);

    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XChangePointerDevice 
 * function, if the client and server have a different byte ordering.
 *
 */

SRepXChangePointerDevice (client, size, rep)
    ClientPtr	client;
    int		size;
    xChangePointerDeviceReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, rep);
    }

#endif /* XINPUT */
