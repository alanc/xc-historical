/* $XConsortium: XIE.h,v 1.3 94/01/12 19:36:23 rws Exp $ */
/*
 * Copyright 1993 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this
 * software without specific, written prior permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $NCDId: @(#)lbxprop.c,v 1.3 1994/02/11 00:09:36 lemke Exp $
 *
 * Author:  Dave Lemke, Network Computing Devices
 */

/* various bits of DIX-level mangling */

#include <sys/types.h>
#include <stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "servermd.h"
#include "propertyst.h"
#include "windowstr.h"
#define _XLBX_SERVER_
#include "lbxstr.h"
#include "lbxserve.h"
#include "lbxtags.h"
#include "Xfuncproto.h"

extern void CopySwap32Write();
extern int (*ProcVector[256])();
extern void (*ReplySwapVector[256]) ();
extern void CopySwap16Write(), CopySwap32Write(), Swap32Write();
extern int fWriteToClient();

int
LbxChangeProperty(client)
    ClientPtr	client;
{
}

int
LbxGetProperty(client)
    ClientPtr	client;
{
    PropertyPtr pProp,
                prevProp;
    unsigned long n,
                len,
                ind;
    WindowPtr   pWin;
    xLbxGetPropertyReply reply;
    TagData     td;
    XID         tid;
    Bool        send_data = FALSE;

    REQUEST(xLbxGetPropertyReq);

    REQUEST_SIZE_MATCH(xLbxGetPropertyReq);
    if (stuff->delete)
	UpdateCurrentTime();
    pWin = (WindowPtr) LookupWindow(stuff->window, client);
    if (pWin) {
	if (!ValidAtom(stuff->property)) {
	    client->errorValue = stuff->property;
	    return (BadAtom);
	}
	if ((stuff->delete != xTrue) && (stuff->delete != xFalse)) {
	    client->errorValue = stuff->delete;
	    return (BadValue);
	}
	if ((stuff->type == AnyPropertyType) || ValidAtom(stuff->type)) {
	    pProp = wUserProps(pWin);
	    prevProp = (PropertyPtr) NULL;
	    while (pProp) {
		if (pProp->propertyName == stuff->property)
		    break;
		prevProp = pProp;
		pProp = pProp->next;
	    }
	    reply.type = X_Reply;
	    reply.sequenceNumber = client->sequence;
	    if (pProp) {

		tid = pProp->tag_id;
		/*
		 * If the request type and actual type don't match. Return the
		 * property information, but not the data.
		 */

		if ((stuff->type != pProp->type) &&
			(stuff->type != AnyPropertyType)) {
		    reply.bytesAfter = pProp->size;
		    reply.format = pProp->format;
		    reply.length = 0;
		    reply.nItems = 0;
		    reply.propertyType = pProp->type;
		    reply.tag = 0;
		    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
		    return (Success);
		}
/* XXX if serve doesn't have data, go get it from owner */
#ifdef morework
if (pProp->owner_pid)
{
suspend()
send query to owner
restart
}
#endif
		/*
		 * Return type, format, value to client
		 */
		n = (pProp->format / 8) * pProp->size;	/* size (bytes) of prop */
		ind = stuff->longOffset << 2;

		/*
		 * If longOffset is invalid such that it causes "len" to be
		 * negative, it's a value error.
		 */

		if (n < ind) {
		    client->errorValue = stuff->longOffset;
		    return BadValue;
		}
		len = min(n - ind, 4 * stuff->longLength);

		if (!tid) {
		    tid = TagNewTag();
		    if (!TagSaveTag(tid, LbxTagTypeProperty, len,
				    ((char *) pProp->data + ind))) {
			tid = 0;
		    }
		    send_data = TRUE;
		} else
		    send_data = !TagProxyMarked(tid, LbxProxyID(client));
		pProp->tag_id = tid;
		TagMarkProxy(tid, LbxProxyID(client));

		reply.bytesAfter = n - (ind + len);
		reply.format = pProp->format;
		if (send_data)
		    reply.length = (len + 3) >> 2;
		else
		    reply.length = 0;
		reply.nItems = len / (pProp->format / 8);
		reply.propertyType = pProp->type;
                reply.tag = pProp->tag_id;

		if (stuff->delete && (reply.bytesAfter == 0)) {
		    LbxFlushPropertyTag(client, tid);
		}
		if (stuff->delete && (reply.bytesAfter == 0)) {	/* send the event */
		    xEvent      event;

		    event.u.u.type = PropertyNotify;
		    event.u.property.window = pWin->drawable.id;
		    event.u.property.state = PropertyDelete;
		    event.u.property.atom = pProp->propertyName;
		    event.u.property.time = currentTime.milliseconds;
		    DeliverEvents(pWin, &event, 1, (WindowPtr) NULL);
		}
		WriteReplyToClient(client, sizeof(xGenericReply), &reply);
		if (send_data && len) {
		    switch (reply.format) {
		    case 32:
			client->pSwapReplyFunc = CopySwap32Write;
			break;
		    case 16:
			client->pSwapReplyFunc = CopySwap16Write;
			break;
		    default:
			client->pSwapReplyFunc = (void (*) ()) fWriteToClient;
			break;
		    }
		    WriteSwappedDataToClient(client, len,
					     (char *) pProp->data + ind);
		}
		if (stuff->delete && (reply.bytesAfter == 0)) {	/* delete the Property */
		    if (prevProp == (PropertyPtr) NULL) {	/* takes care of head */
			if (!(pWin->optional->userProps = pProp->next))
			    CheckWindowOptionalNeed(pWin);
		    } else
			prevProp->next = pProp->next;
		    TagDeleteTag(tid);
		    xfree(pProp->data);
		    xfree(pProp);
		}
	    } else {
		reply.nItems = 0;
		reply.length = 0;
		reply.bytesAfter = 0;
		reply.propertyType = None;
		reply.format = 0;
		reply.tag = 0;
		WriteReplyToClient(client, sizeof(xGenericReply), &reply);
	    }
	    return (client->noClientException);

	} else {
	    client->errorValue = stuff->type;
	    return (BadAtom);
	}
    } else
	return (BadWindow);
}

int
LbxPropertyData(client)
    ClientPtr	client;
{
    PropertyPtr prop;
    TagData     td;
    pointer     data;
    int         len;

    REQUEST(xLbxPropertyDataReq);

    data = (pointer) &stuff[1];

    td = TagGetTag(stuff->tag);
    if (td) {			/* may have been destroyed out from under us */
	prop = (PropertyPtr) td->tdata;
        len = prop->size * prop->format/8;
	bcopy((char *) data, (char *) prop->data, len);
    }
    return client->noClientException;
}


int
LbxFlushPropertyTag(tid)
    XID	tid;
{
    LbxSendInvalidateTagToProxies(tid, LbxTagTypeProperty);
    TagDeleteTag(tid);
}
