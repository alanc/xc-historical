#ifdef XINPUT
/* $XConsortium: xopendev.c,v 1.6 89/10/12 12:22:10 gms Exp $ */

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
 * Request to open an extension input device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "XI.h"
#include "XIproto.h"
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "windowstr.h"			/* window structure  */

extern	int 		IReqCode;
extern	int 		BadDevice;
extern	CARD8		event_base [];
extern	InputInfo	inputInfo;
extern	void		(* ReplySwapVector[256]) ();
DeviceIntPtr		LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure swaps the request if the server and client have different
 * byte orderings.
 *
 */

int
SProcXOpenDevice(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xOpenDeviceReq);
    swaps(&stuff->length, n);
    return(ProcXOpenDevice(client));
    }

/***********************************************************************
 *
 * This procedure causes the server to open an input device.
 *
 */

int
ProcXOpenDevice(client)
    register ClientPtr client;
    {
    xInputClassInfo evbase [numInputClasses];
    int j=0;
    int status = Success;
    xOpenDeviceReply	rep;
    DeviceIntPtr dev;
    void OpenInputDevice();

    REQUEST(xOpenDeviceReq);
    REQUEST_SIZE_MATCH(xOpenDeviceReq);

    if (stuff->deviceid == inputInfo.pointer->id || 
	stuff->deviceid == inputInfo.keyboard->id)
	{
	SendErrorToClient(client, IReqCode, X_OpenDevice, 0, BadDevice);
        return Success;
	}

    if ((dev = LookupDeviceIntRec(stuff->deviceid)) == NULL) /* not open */
	{
        for (dev=inputInfo.off_devices; dev; dev=dev->next)
	    if (dev->id == stuff->deviceid)
		break;
	if (dev == NULL)
	    status = BadDevice;
	else
	    {
	    OpenInputDevice (dev, client, &status);
	    if (dev->inited && dev->startup)
		(void)EnableDevice(dev);
	    }
	}

    if (status != Success)
	{
	SendErrorToClient(client, IReqCode, X_OpenDevice, 0, status);
	return Success;
	}

    rep.repType = X_Reply;
    rep.RepType = X_OpenDevice;
    rep.sequenceNumber = client->sequence;
    if (dev->key != NULL)
	{
	evbase[j].class = KeyClass;
	evbase[j++].event_type_base = event_base[KeyClass];
	}
    if (dev->button != NULL)
	{
	evbase[j].class = ButtonClass;
	evbase[j++].event_type_base = event_base[ButtonClass];
	}
    if (dev->valuator != NULL)
	{
	evbase[j].class = ValuatorClass;
	evbase[j++].event_type_base = event_base[ValuatorClass];
	}
    if (dev->focus != NULL)
	{
	evbase[j].class = FocusClass;
	evbase[j++].event_type_base = event_base[FocusClass];
	}
    if (dev->proximity != NULL)
	{
	evbase[j].class = ProximityClass;
	evbase[j++].event_type_base = event_base[ProximityClass];
	}
    evbase[j].class = OtherClass;
    evbase[j++].event_type_base = event_base[OtherClass];
    rep.length = (j * sizeof (xInputClassInfo) + 3) >> 2;
    rep.num_classes = j;
    WriteReplyToClient (client, sizeof (xOpenDeviceReply), &rep);
    WriteToClient(client, j * sizeof (xInputClassInfo), evbase);
    return (Success);
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XOpenDevice function,
 * if the client and server have a different byte ordering.
 *
 */

SRepXOpenDevice (client, size, rep)
    ClientPtr	client;
    int		size;
    xOpenDeviceReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, rep);
    }

#endif /* XINPUT */
