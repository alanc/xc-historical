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
 * $NCDId: @(#)lbxdix.c,v 1.11 1994/02/11 00:10:55 lemke Exp $
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
#include "dixfontstr.h"
#include "gcstruct.h"
#define _XLBX_SERVER_
#include "lbxstr.h"
#include "lbxserve.h"
#include "lbxtags.h"
#include "Xfuncproto.h"

extern void CopySwap32Write();
extern int (*ProcVector[256])();
extern void (*ReplySwapVector[256]) ();

/* XXX should be per-proxy */
static int  motion_allowed_events = 0;

static int	lbx_font_private;

void
LbxDixInit()
{
    TagInit();
    lbx_font_private = AllocateFontPrivateIndex();
}

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
	LbxSendInvalidateTagToProxies(modifier_map_tag, LbxTagTypeModmap);
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
	LbxSendInvalidateTagToProxies(keyboard_map_tag, LbxTagTypeKeymap);
	keyboard_map_tag = 0;
    }
}

int
LbxQueryFont(client)
    ClientPtr	client;
{
    xQueryFontReply *reply;
    xLbxQueryFontReply lbxrep;
    FontPtr     pFont;
    register GC *pGC;
    Bool        queried_info = FALSE;
    Bool        send_data = FALSE;
    Bool        free_reply = FALSE;
    int         rlength = 0;
    TagData     td;
    XID         tid;

    REQUEST(xLbxQueryFontReq);

    REQUEST_SIZE_MATCH(xLbxQueryFontReq);

    client->errorValue = stuff->fid;	/* EITHER font or gc */
    pFont = (FontPtr) LookupIDByType(stuff->fid, RT_FONT);
    if (!pFont) {
	/* can't use VERIFY_GC because it might return BadGC */
	pGC = (GC *) LookupIDByType(stuff->fid, RT_GC);
	if (!pGC || !pGC->font) {	/* catch a non-existent builtin font */
	    client->errorValue = stuff->fid;
	    return (BadFont);	/* procotol spec says only error is BadFont */
	}
	pFont = pGC->font;
    }
    /* get tag (if any) */
    td = (TagData) FontGetPrivate(pFont, lbx_font_private);

    if (!td) {
	xCharInfo  *pmax = FONTINKMAX(pFont);
	xCharInfo  *pmin = FONTINKMIN(pFont);
	int         nprotoxcistructs;

	nprotoxcistructs = (
			  pmax->rightSideBearing == pmin->rightSideBearing &&
			    pmax->leftSideBearing == pmin->leftSideBearing &&
			    pmax->descent == pmin->descent &&
			    pmax->ascent == pmin->ascent &&
			    pmax->characterWidth == pmin->characterWidth) ?
	    0 : N2dChars(pFont);

	rlength = sizeof(xQueryFontReply) +
	    FONTINFONPROPS(FONTCHARSET(pFont)) * sizeof(xFontProp) +
	    nprotoxcistructs * sizeof(xCharInfo);
	reply = (xQueryFontReply *) xalloc(rlength);
	if (!reply) {
	    return (BadAlloc);
	}
	queried_info = TRUE;
	send_data = TRUE;
	QueryFont(pFont, reply, nprotoxcistructs);
    } else {			/* just get data from tag */
	reply = (xQueryFontReply *) td->tdata;
	rlength = td->size;
    }

    if (!td) {
	/* data allocation is done when font is first queried */
	tid = TagNewTag();
	if (TagSaveTag(tid, LbxTagTypeFont, rlength, (pointer) reply)) {
	    td = TagGetTag(tid);
	    FontSetPrivate(pFont, lbx_font_private, (pointer) td);
	} else {		/* can't save it for later, so be sure to
				 * clean up */
	    free_reply = TRUE;
	}
    }
    if (td) {
	TagMarkProxy(td->tid, LbxProxyID(client));
	lbxrep.tag = td->tid;
    } else {
	lbxrep.tag = 0;
	send_data = TRUE;
    }

    lbxrep.type = X_Reply;
    lbxrep.sequenceNumber = client->sequence;
    if (send_data)
	lbxrep.length = rlength >> 2;
    else
	lbxrep.length = 0;

    WriteToClient(client, sizeof(xLbxQueryFontReply), (char *) &lbxrep);
/* XXX swapping nastiness here -- if this swaps stuff, the stashed copy
 * gets munged...
 */
    if (send_data)
	WriteReplyToClient(client, rlength, reply);
    if (free_reply)
	xfree(reply);
    return (client->noClientException);
}

void
LbxFreeFontTag(pfont)
    FontPtr	pfont;
{
    TagData	td;

    td = (TagData) FontGetPrivate(pfont, lbx_font_private);
    if (td) {
	LbxSendInvalidateTagToProxies(td->tid, LbxTagTypeFont);
        TagDeleteTag(td->tid);
    }
}

int
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

LbxSendInvalidateTag(client, tag, tagtype)
    ClientPtr	client;
    XID         tag;
    int		tagtype;
{
    xLbxEvent   ev;
    int         n;
    extern int  LbxEventCode;

    ev.type = LbxEventCode;
    ev.lbxType = LbxInvalidateTagEvent;
    ev.sequenceNumber = client->sequence;
    ev.client = client->index;
    ev.detail1 = tag;
    ev.detail2 = tagtype;

    if (client->swapped) {
	swaps(&ev.sequenceNumber, n);
	swapl(&ev.client, n);
    }
    DBG (DBG_CLIENT, (stderr, "Invalidating tag  %d\n", tag));
    WriteToClient(client, sizeof(xLbxEvent), (char *) &ev);
}


LbxSendInvalidateTagToProxies(tag, tagtype)
    XID		tag;
    int		tagtype;
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
	    LbxSendInvalidateTag(client, tag, tagtype);
            TagClearProxy(tag, LbxProxyID(client));
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
    /* XXX flush all font tags -- how? */
    /* without this, a proxy re-connect will confuse things slightly -- the
     * proxy will need to do QueryTag to get in sync on font data & global
     * props
     */
}

/* when server resets, need to reset global tags */
LbxResetTags()
{
    modifier_map_tag = 0;
    keyboard_map_tag = 0;
}
