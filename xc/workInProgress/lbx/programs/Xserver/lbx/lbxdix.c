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
 * $NCDId: @(#)lbxdix.c,v 1.7 1994/02/02 02:06:06 lemke Exp $
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
#include "colormapst.h"
#include "resource.h"
#include "inputstr.h"
#include "servermd.h"
#define _XLBX_SERVER_
#include "lbxstr.h"
#include "lbxserve.h"
#include "lbxtags.h"
#include "Xfuncproto.h"

extern void CopySwap32Write();
extern int (*ProcVector[256])();

/* XXX should be per-proxy */
static int  motion_allowed_events = 0;

void
LbxAllowMotion(client, num)
{
    motion_allowed_events += num;
}

Bool
LbxThrottleMotionEvents(client, ev)
    ClientPtr   client;
    xEvent     *ev;
{
    /* XXX use client to check proxy */
    if (ev->u.u.type == MotionNotify) {
	if (motion_allowed_events == 0) {
	    DBG(DBG_CLIENT, (stderr, "throttling motion event for client %d\n", client->index));
	    return TRUE;
	} else {
	    motion_allowed_events--;
	}
    }
    return FALSE;
}

/*
 * this looks up the color associated with the pixel, and then calls
 * AllocColor()  a bit round-about, but it should work.
 */
void
LbxIncrementPixel(client, cmap, pixel, amount)
    int         client;
    Colormap    cmap;
    Pixel       pixel;
    int         amount;
{
    ColormapPtr pmap;
    EntryPtr    pent;
    unsigned short red,
                green,
                blue;
    int         i;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap)
	return;
    switch (pmap->class) {
    case StaticColor:
    case StaticGray:
	red = pmap->red[pixel].co.local.red;
	green = pmap->red[pixel].co.local.green;
	blue = pmap->red[pixel].co.local.blue;
	break;
    case GrayScale:
    case PseudoColor:
	pent = pmap->red + pixel;
	red = pent->co.local.red;
	green = pent->co.local.green;
	blue = pent->co.local.blue;
	break;
    default:
/* XXX */
	return;
    }
    for (i = 0; i < amount; i++)
	AllocColor(pmap, &red, &green, &blue, &pixel, client);
}

extern InputInfo inputInfo;

static int  modifier_map_tag;

int
LbxGetModifierMapping(client)
    ClientPtr   client;
{
    TagData     td;
    pointer     tagdata;
    xLbxGetModifierMappingReply rep;
    register KeyClassPtr keyc = inputInfo.keyboard->key;
    int         dlength = keyc->maxKeysPerModifier << 3;
    Bool        tag_known = FALSE,
                send_data;
    int         n;

    if (!modifier_map_tag) {
	modifier_map_tag = TagNewTag();
	tagdata = (pointer) keyc->modifierKeyMap;
	if (!TagSaveTag(modifier_map_tag, LbxTagTypeModmap, dlength,
			tagdata)) {
	    /* can't save it, so report no tag */
	    modifier_map_tag = 0;
	}
    } else {
	td = TagGetTag(modifier_map_tag);
        tagdata = td->tdata;
	tag_known = TagProxyMarked(modifier_map_tag, LbxProxyID(client));
    }
    if (modifier_map_tag)
	TagMarkProxy(modifier_map_tag, LbxProxyID(client));

    send_data = (!modifier_map_tag || !tag_known);

    rep.type = X_Reply;
    rep.keyspermod = keyc->maxKeysPerModifier;
    rep.sequenceNumber = client->sequence;
    rep.tag = modifier_map_tag;
    if (send_data)
	rep.length = dlength >> 2;
    else
	rep.length = 0;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.tag, n);
    }
    WriteToClient(client, sizeof(xLbxGetModifierMappingReply), &rep);

    if (send_data)
	WriteToClient(client, dlength, (char *) tagdata);

    return client->noClientException;
}

LbxFlushModifierMapTag()
{

    if (modifier_map_tag) {
	TagDeleteTag(modifier_map_tag);
	LbxSendInvalidateTagToProxies(modifier_map_tag);
	modifier_map_tag = 0;
    }
}

static int  keyboard_map_tag;

int
LbxGetKeyboardMapping(client)
    ClientPtr   client;
{
    TagData     td;
    pointer     tagdata;
    xLbxGetKeyboardMappingReply rep;
    REQUEST(xLbxGetKeyboardMappingReq);
    KeySymsPtr curKeySyms = &inputInfo.keyboard->key->curKeySyms;
    int         dlength;
    Bool        tag_known = FALSE,
                send_data;
    int         n;

    REQUEST_SIZE_MATCH(xLbxGetKeyboardMappingReq);

/* XXX do we need this?  proxy should be smart enough to do this right
 * all the time
 */
    if ((stuff->firstKeyCode < curKeySyms->minKeyCode) ||
        (stuff->firstKeyCode > curKeySyms->maxKeyCode)) {
	client->errorValue = stuff->firstKeyCode;
	return BadValue;
    }
    if (stuff->firstKeyCode + stuff->count > curKeySyms->maxKeyCode + 1) {
	client->errorValue = stuff->count;
        return BadValue;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.keysperkeycode = curKeySyms->mapWidth;
    /* length is a count of 4 byte quantities and KeySyms are 4 bytes */

    if (!keyboard_map_tag) {
	keyboard_map_tag = TagNewTag();
	tagdata = (pointer) curKeySyms->map[(stuff->firstKeyCode -
		curKeySyms->minKeyCode) * curKeySyms->mapWidth];
	dlength = (curKeySyms->mapWidth * stuff->count);
	if (!TagSaveTag(keyboard_map_tag, LbxTagTypeKeymap, dlength,
			tagdata)) {
	    /* can't save it, so report no tag */
	    keyboard_map_tag = 0;
	}
    } else {
	td = TagGetTag(keyboard_map_tag);
        tagdata = td->tdata;
	tag_known = TagProxyMarked(keyboard_map_tag, LbxProxyID(client));
    }
    if (keyboard_map_tag)
	TagMarkProxy(keyboard_map_tag, LbxProxyID(client));

    send_data = (!keyboard_map_tag || !tag_known);

    rep.type = X_Reply;
    rep.keysperkeycode = curKeySyms->mapWidth;
    rep.sequenceNumber = client->sequence;
    rep.tag = keyboard_map_tag;
    if (send_data)
	rep.length = (curKeySyms->mapWidth * stuff->count);
    else
	rep.length = 0;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.tag, n);
    }
    WriteToClient(client, sizeof(xLbxGetKeyboardMappingReply), &rep);

    if (send_data) {
	client->pSwapReplyFunc = CopySwap32Write;
	WriteSwappedDataToClient(
	    client,
	    curKeySyms->mapWidth * stuff->count * sizeof(KeySym),
	    &curKeySyms->map[(stuff->firstKeyCode - curKeySyms->minKeyCode) *
			     curKeySyms->mapWidth]);
    }
    return client->noClientException;
}

LbxFlushKeyboardMapTag()
{

    if (keyboard_map_tag) {
	TagDeleteTag(keyboard_map_tag);
	LbxSendInvalidateTagToProxies(keyboard_map_tag);
	keyboard_map_tag = 0;
    }
}

LbxQueryTag(client, tag)
    ClientPtr   client;
    XID         tag;
{
    xLbxQueryTagReply rep;
    TagData     tdata;
    int         n;

    tdata = TagGetTag(tag);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    if (!tdata) {
	rep.valid = 0;
	rep.length = 0;
    } else {
	rep.valid = 1;
	rep.length = tdata->size >> 2;
    }
    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xLbxQueryTagReply), (char *) &rep);
    if (tdata)
	WriteToClient(client, tdata->size, (char *) tdata->tdata);

    return client->noClientException;
}

LbxInvalidateTag(client, tag)
    ClientPtr   client;
    XID         tag;
{
    TagClearProxy(tag, LbxProxyID(client));
    return client->noClientException;
}

LbxSendInvalidateTag(client, tag)
    ClientPtr	client;
    XID         tag;
{
    xLbxEvent   ev;
    int         n;
    extern int  LbxEventCode;

    ev.type = LbxEventCode;
    ev.lbxType = LbxInvalidateTagEvent;
    ev.sequenceNumber = client->sequence;
    ev.client = client->index;
    ev.detail = tag;

    if (client->swapped) {
	swaps(&ev.sequenceNumber, n);
	swapl(&ev.client, n);
    }
    DBG (DBG_CLIENT, (stderr, "Invalidating tag  %d\n", tag));
    WriteToClient(client, sizeof(xLbxEvent), (char *) &ev);
}


LbxSendInvalidateTagToProxies(tag)
    XID		tag;
{
    LbxProxyPtr	proxy;
    int		i;
    ClientPtr	client;
    extern LbxProxyPtr	proxyList;

    /* send Invalidates to all proxies */
    proxy = proxyList;
    while (proxy) {
	/* find some client of the proxy to use */
	if (client = proxy->lbxClients[LbxMasterClientIndex]->client) {
	    LbxSendInvalidateTag(client, tag);
	}
	proxy = proxy->next;
    }
}

/*
 * clear out markers for proxies
 */
LbxFlushTags(proxy)
    LbxProxyPtr proxy;
{
    if (modifier_map_tag)
	TagClearProxy(modifier_map_tag, proxy->pid);
    if (keyboard_map_tag)
	TagClearProxy(keyboard_map_tag, proxy->pid);
}

int
LbxDecodePoly(client, xreqtype, decode_rtn)
    register ClientPtr  client;
    CARD8		xreqtype;
    int			(*decode_rtn)();
{
    REQUEST(xLbxPolyPointReq);
    char		*in;
    char		*inend;
    xPolyPointReq	*xreq;
    int			len;
    int			retval;

    len = (stuff->length << 2) - sz_xLbxPolyPointReq;
    if ((xreq = (xPolyPointReq *) 
	    xalloc(sizeof(xPolyPointReq) + (len << 1))) == NULL)
	return BadAlloc;
    in = (char *)stuff + sz_xLbxPolyPointReq;
    len = (*decode_rtn)(in, in + len - stuff->padBytes, &xreq[1]);
    xreq->reqType = xreqtype;
    xreq->coordMode = 1;
    xreq->drawable = stuff->drawable;
    xreq->gc = stuff->gc;
    client->req_len = xreq->length = (sizeof(xPolyPointReq) + len) >> 2;
    client->requestBuffer = (char *)xreq;

    retval = (*ProcVector[xreqtype])(client);
    xfree(xreq);
    return retval;
}

int
LbxDecodeFillPoly(client)
    register ClientPtr  client;
{
    REQUEST(xLbxFillPolyReq);
    char		*in;
    char		*inend;
    xFillPolyReq	*xreq;
    int			len;
    int			retval;

    len = (stuff->length << 2) - sz_xLbxFillPolyReq;
    if ((xreq = (xFillPolyReq *) 
	    xalloc(sizeof(xFillPolyReq) + (len << 1))) == NULL)
	return BadAlloc;
    in = (char *)stuff + sz_xLbxFillPolyReq;
    len = LbxDecodePoints(in, in + len - stuff->padBytes, &xreq[1]);
    xreq->reqType = X_FillPoly;
    xreq->drawable = stuff->drawable;
    xreq->gc = stuff->gc;
    xreq->shape = stuff->shape;
    xreq->coordMode = 1;
    client->req_len = xreq->length = (sizeof(xFillPolyReq) + len) >> 2;
    client->requestBuffer = (char *)xreq;

    retval = (*ProcVector[X_FillPoly])(client);
    xfree(xreq);
    return retval;
}

/*
 * Routines for decoding line drawing requests
 */

#define DECODE_SHORT(in, val) \
    if ((*(in) & 0xf0) != 0x80) \
        (val) = (short)*(in)++; \
    else { \
	(val) = ((short)(*(in) & 0x0f) << 8) | (unsigned char)*((in) + 1); \
	if (*(in) & 0x08) (val) |= 0xf000; \
	(in) += 2; \
    }

#define DECODE_USHORT(in, val) \
    if ((*(in) & 0xf0) != 0xf0) \
	(val) = (unsigned short)*(in)++; \
    else { \
	(val) = ((short)(*(in) & 0x0f) << 8) | (unsigned char)*((in) + 1); \
	(in) += 2; \
    }

#define DECODE_ANGLE(in, val) \
    if ((*(in) & 0xf0) == 0x80) \
	(val) = (((short)(*(in)++ & 0x0f) - 4) * 45) << 7; \
    else { \
	(val) = ((short)*(in) << 8) | (unsigned char)*((in) + 1); \
	(in) += 2; \
    }

int
LbxDecodePoints(in, inend, out)
    register char  *in;
    char	   *inend;
    register short *out;
{
    char	   *start_out = (char *)out;

    while (in < inend) {
	DECODE_SHORT(in, *out);
	out++;
	DECODE_SHORT(in, *out);
	out++;
    }
    return ((char *)out - start_out);
}

int
LbxDecodeSegment(in, inend, out)
    register char  *in;
    char	   *inend;
    register short *out;
{
    register short diff;
    short	   last_x = 0;
    short	   last_y = 0;
    char	   *start_out = (char *)out;

    while (in < inend) {
	DECODE_SHORT(in, diff);
	*out = last_x + diff;
	last_x += diff;
	out++;
	DECODE_SHORT(in, diff);
	*out = last_y + diff;
	last_y += diff;
	out++;

	DECODE_SHORT(in, diff);
	*out = last_x + diff;
	out++;
	DECODE_SHORT(in, diff);
	*out = last_y + diff;
	out++;
    }
    return ((char *)out - start_out);
}

int
LbxDecodeRectangle(in, inend, out)
    register char  *in;
    char	   *inend;
    register short *out;
{
    register short diff;
    unsigned short len;
    short	   last_x = 0;
    short	   last_y = 0;
    char	   *start_out = (char *)out;

    while (in < inend) {
	DECODE_SHORT(in, diff);
	*out = last_x + diff;
	last_x += diff;
	out++;
	DECODE_SHORT(in, diff);
	*out = last_y + diff;
	last_y += diff;
	out++;

	DECODE_USHORT(in, *(unsigned short *)out);
	out++;
	DECODE_USHORT(in, *(unsigned short *)out);
	out++;
    }
    return ((char *)out - start_out);
}

int
LbxDecodeArc(in, inend, out)
    register char  *in;
    char	   *inend;
    register short *out;
{
    register short diff;
    short	   last_x = 0;
    short	   last_y = 0;
    char	   *start_out = (char *)out;

    while (in < inend) {
	DECODE_SHORT(in, diff);
	*out = last_x + diff;
	last_x += diff;
	out++;
	DECODE_SHORT(in, diff);
	*out = last_y + diff;
	last_y += diff;
	out++;

	DECODE_USHORT(in, *(unsigned short *)out);
	out++;
	DECODE_USHORT(in, *(unsigned short *)out);
	out++;

	DECODE_ANGLE(in, *out);
	out++;
	DECODE_ANGLE(in, *out);
	out++;
    }
    return ((char *)out - start_out);
}
