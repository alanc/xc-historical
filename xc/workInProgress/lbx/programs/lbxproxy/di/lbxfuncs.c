/*
 * Copyright 1994 Network Computing Devices, Inc.
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
 * $NCDId: @(#)lbxfuncs.c,v 1.23 1994/02/11 21:13:48 lemke Exp $
 */

/* $XConsortium: lbxfuncs.c,v 1.2 94/02/10 20:08:59 dpw Exp $ */

/*
 * top level LBX request & reply handling
 */


#include	<stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include	<X11/X.h>	/* for KeymapNotify */
#include	<X11/Xproto.h>
#include	"assert.h"
#include	"lbxdata.h"
#include	"atomcache.h"
#include	"util.h"
#include	"tags.h"
#include	"colormap.h"
#include	"cmapst.h"
#include	"lbx.h"		/* gets dixstruct.h */
#include	"resource.h"
#include	"wire.h"
#define _XLBX_SERVER_
#include	"lbxstr.h"	/* gets dixstruct.h */

#define reply_length(cp,rep) ((rep)->type==X_Reply ? \
        32 + (HostUnswapLong((cp),(rep)->length) << 2) \
	: 32)

#define	server_resource(client, xid)	(CLIENT_ID(xid) == 0)

static int
intern_atom_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xInternAtomReq *req;
    char       *s;
    Atom        atom,
                a;
    int         len;
    xInternAtomReply reply;
    ReplyStuffPtr nr;

    req = (xInternAtomReq *) data;

    if (req->nbytes > MAX_ATOM_LENGTH)
	return REQ_NOCHANGE;

    len = req->nbytes + sizeof(xInternAtomReq);

    s = data + sizeof(xInternAtomReq);

    atom = MakeAtom(s, req->nbytes, &a, FALSE);
    if (atom != None) {
	/* found preset atom */
	if (LBXCacheSafe(client)) {
	    reply.type = X_Reply;
	    reply.length = 0;
	    reply.sequenceNumber = LBXSequenceNumber(client);
	    reply.atom = atom;
	    WriteToClient(client, sizeof(xInternAtomReply), &reply);

#ifdef LBX_STATS
	    intern_good++;
#endif

	    return REQ_YANK;
	}
    } else if (req->nbytes < MAX_ATOM_LENGTH) {
	nr = NewReply(client);
	if (nr) {
	    strncpy(nr->request_info.lbxatom.str, s, req->nbytes);
	    nr->request_info.lbxatom.len = req->nbytes;
	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = X_InternAtom;
	}
    }

#ifdef LBX_STATS
    intern_miss++;
#endif

    return REQ_NOCHANGE;
}

static Bool
intern_atom_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    Atom        atom;
    char       *str;
    xInternAtomReply *reply;
    int         len;
    ReplyStuffPtr nr;

    reply = (xInternAtomReply *) data;

    atom = reply->atom;
    nr = GetReply(client);
    assert(nr);
    str = nr->request_info.lbxatom.str;
    len = nr->request_info.lbxatom.len;

    if (atom != None) {
	/* make sure it gets stuffed in the DB */
	(void) MakeAtom(str, len, atom, TRUE);
    }
    return TRUE;
}

static int
get_atom_name_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xResourceReq *req;
    char       *str;
    xGetAtomNameReply reply;
    int         len;
    ReplyStuffPtr nr;

    req = (xResourceReq *) data;

    str = NameForAtom(req->id);

    if (str) {
	/* found the value */
	if (LBXCacheSafe(client)) {

	    len = strlen(str);
	    reply.type = X_Reply;
	    reply.length = (len + 3) >> 2;
	    reply.sequenceNumber = LBXSequenceNumber(client);
	    reply.nameLength = len;
	    WriteToClient(client, sizeof(xGetAtomNameReply), &reply);
	    WriteToClientPad(client, len, str);

#ifdef LBX_STATS
	    getatom_good++;
#endif

	    return REQ_YANK;
	}
    } else {
	nr = NewReply(client);
	if (nr) {
	    nr->request_info.lbxatom.atom = req->id;
	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = X_GetAtomName;
	}
    }

#ifdef LBX_STATS
    getatom_miss++;
#endif

    return REQ_NOCHANGE;
}

static Bool
get_atom_name_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    Atom        atom;
    char       *s;
    xGetAtomNameReply *reply;
    int         len;
    ReplyStuffPtr nr;

    reply = (xGetAtomNameReply *) data;

    if ((reply->length << 2) > MAX_ATOM_LENGTH)
	return;

    len = (reply->length << 2) + sizeof(xGetAtomNameReply);

    s = data + sizeof(xGetAtomNameReply);

    len -= sizeof(xGetAtomNameReply);
    nr = GetReply(client);
    assert(nr);
    atom = nr->request_info.lbxatom.atom;

    /* make sure it gets stuffed in the DB */
    (void) MakeAtom(s, len, atom, TRUE);
    return TRUE;
}

static int
lookup_color_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xLookupColorReq *req;
    xLookupColorReply reply;
    ReplyStuffPtr nr;
    RGBEntryPtr rgbe;
    int         len;

    req = (xLookupColorReq *) data;

    len = req->nbytes;
    if (len > MAX_COLORNAME_LENGTH)
	return REQ_NOCHANGE;

/* XXX for now, ignore attempts to anything but default colormaps */
    if (!server_resource(client, req->cmap))
	return REQ_NOCHANGE;

    rgbe = FindColorName((char *) &req[1], len, req->cmap);

    if (rgbe) {
	/* found the value */
	if (LBXCacheSafe(client)) {
	    reply.type = X_Reply;
	    reply.length = 0;
	    reply.sequenceNumber = LBXSequenceNumber(client);

	    reply.exactRed = rgbe->xred;
	    reply.exactBlue = rgbe->xblue;
	    reply.exactGreen = rgbe->xgreen;

	    reply.screenRed = rgbe->vred;
	    reply.screenBlue = rgbe->vblue;
	    reply.screenGreen = rgbe->vgreen;

	    WriteToClient(client, sizeof(xLookupColorReply), &reply);

#ifdef LBX_STATS
	    luc_good++;
#endif

	    return REQ_YANK;
	}
    } else {
	nr = NewReply(client);
	if (nr) {
	    strncpy(nr->request_info.lbxlookupcolor.name,
		    (char *) &req[1], len);
	    nr->request_info.lbxlookupcolor.namelen = len;
	    nr->request_info.lbxlookupcolor.cmap = req->cmap;
	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = X_LookupColor;
	}
    }

#ifdef LBX_STATS
    luc_miss++;
#endif

    return REQ_NOCHANGE;
}

static Bool
lookup_color_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xLookupColorReply *reply;
    int         len;
    ReplyStuffPtr nr;
    RGBEntryRec rgbe;

    reply = (xLookupColorReply *) data;

    nr = GetReply(client);
    assert(nr);

    rgbe.xred = reply->exactRed;
    rgbe.xblue = reply->exactBlue;
    rgbe.xgreen = reply->exactGreen;
    rgbe.vred = reply->screenRed;
    rgbe.vblue = reply->screenBlue;
    rgbe.vgreen = reply->screenGreen;

    rgbe.cmap = nr->request_info.lbxlookupcolor.cmap;

    AddColorName(nr->request_info.lbxlookupcolor.name,
		 nr->request_info.lbxlookupcolor.namelen,
		 &rgbe);
    return TRUE;
}

static int
alloc_color_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xAllocColorReq *req;
    xAllocColorReply reply;
    Pixel       pix;
    Entry      *pent;
    ReplyStuffPtr nr;

    req = (xAllocColorReq *) data;

    FindPixel(client, req->cmap, (int) req->red, (int) req->green,
	      (int) req->blue, &pent);
    if (pent) {
	/* always inc the pixel, so our refcounts match the server's */
	IncrementPixel(client, req->cmap, pent);

	/* found the value */
	if (LBXCacheSafe(client)) {

	    /* must tell server to bump refcnt */
	    SendIncrementPixel(client, req->cmap, pent->pixel);

	    reply.type = X_Reply;
	    reply.length = 0;
	    reply.sequenceNumber = LBXSequenceNumber(client);

	    reply.red = pent->rep_red;
	    reply.green = pent->rep_green;
	    reply.blue = pent->rep_blue;
	    reply.pixel = pent->pixel;

	    WriteToClient(client, sizeof(xAllocColorReply), &reply);

#ifdef LBX_STATS
	    ac_good++;
#endif

	    return REQ_REPLACE;	/* packet sent anyways */
	}
    } else {
	nr = NewReply(client);
	if (nr) {
	    nr->request_info.lbxalloccolor.cmap = req->cmap;
	    nr->request_info.lbxalloccolor.red = req->red;
	    nr->request_info.lbxalloccolor.green = req->green;
	    nr->request_info.lbxalloccolor.blue = req->blue;
	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = X_AllocColor;
	}
    }

#ifdef LBX_STATS
    ac_miss++;
#endif

    return REQ_NOCHANGE;
}

static Bool
alloc_color_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xAllocColorReply *reply;
    ReplyStuffPtr nr;

    reply = (xAllocColorReply *) data;

    nr = GetReply(client);
    assert(nr);

    /*
     * save requested color, since they're more likely to ask for the same
     * thing again.  also have to store reply RGB.
     */
    StorePixel(client, nr->request_info.lbxalloccolor.cmap,
	       nr->request_info.lbxalloccolor.red,
	       nr->request_info.lbxalloccolor.green,
	       nr->request_info.lbxalloccolor.blue,
	       (int) reply->red, (int) reply->green, (int) reply->blue,
	       reply->pixel);
    return TRUE;
}

static int
alloc_named_color_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xAllocNamedColorReq *req;
    xAllocNamedColorReply reply;
    Pixel       pix;
    Entry      *pent;
    RGBEntryPtr rgbe;
    ReplyStuffPtr nr;

    req = (xAllocNamedColorReq *) data;

    if (req->nbytes > MAX_COLORNAME_LENGTH)
	return REQ_NOCHANGE;

    FindNamedPixel(client, req->cmap, (char *) &req[1], req->nbytes, &pent);
    rgbe = FindColorName((char *) &req[1], req->nbytes, req->cmap);

    /* better get both if we got one... */
    if (pent && rgbe) {
	/* always inc the pixel, so our refcounts match the server's */
	IncrementPixel(client, req->cmap, pent);
	/* found the value */
	if (LBXCacheSafe(client)) {

	    /* must tell server to bump refcnt */
	    SendIncrementPixel(client, req->cmap, pent->pixel);

	    reply.type = X_Reply;
	    reply.length = 0;
	    reply.sequenceNumber = LBXSequenceNumber(client);

	    reply.screenRed = pent->rep_red;
	    reply.screenGreen = pent->rep_green;
	    reply.screenBlue = pent->rep_blue;
	    reply.exactRed = rgbe->xred;
	    reply.exactGreen = rgbe->xgreen;
	    reply.exactBlue = rgbe->xblue;

	    reply.pixel = pent->pixel;

	    WriteToClient(client, sizeof(xAllocNamedColorReply), &reply);

#ifdef LBX_STATS
	    anc_good++;
#endif

	    return REQ_REPLACE;	/* packet sent anyways */
	}
    } else {
	nr = NewReply(client);
	if (nr) {
	    nr->request_info.lbxallocnamedcolor.cmap = req->cmap;
	    strncpy(nr->request_info.lbxallocnamedcolor.name, (char *) &req[1],
		    req->nbytes);
	    nr->request_info.lbxallocnamedcolor.namelen = req->nbytes;

	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = X_AllocNamedColor;
	}
    }

#ifdef LBX_STATS
    anc_miss++;
#endif

    return REQ_NOCHANGE;
}

static Bool
alloc_named_color_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xAllocNamedColorReply *reply;
    ReplyStuffPtr nr;

    reply = (xAllocNamedColorReply *) data;

    nr = GetReply(client);
    assert(nr);

    StoreNamedPixel(client,
		    nr->request_info.lbxallocnamedcolor.cmap,
		    nr->request_info.lbxallocnamedcolor.name,
		    nr->request_info.lbxallocnamedcolor.namelen,
      (int) reply->exactRed, (int) reply->exactGreen, (int) reply->exactBlue,
    (int) reply->screenRed, (int) reply->screenGreen, (int) reply->screenBlue,
		    reply->pixel);
    return TRUE;
}

static int
free_colormap_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xResourceReq *req;

    req = (xResourceReq *) data;
    FreeColormap(req->id);

    return REQ_NOCHANGE;
}

static int
create_colormap_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xCreateColormapReq *req;

    req = (xCreateColormapReq *) data;
    if (!req->alloc)		/* AllocAll are read/write, so ignore */
	CreateColormap(client, req->mid, req->window, req->visual);

    return REQ_NOCHANGE;
}

static int
copy_colormap_and_free_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xCopyColormapAndFreeReq *req;

    req = (xCopyColormapAndFreeReq *) data;
    CopyAndFreeColormap(client, req->mid, req->srcCmap);

    return REQ_NOCHANGE;
}

static int
free_colors_req(client, data)
    ClientPtr   client;
    char       *data;
{
    xFreeColorsReq *req;
    int         num;
    Pixel      *pixels;

    req = (xFreeColorsReq *) data;

    num = ((req->length << 2) - sizeof(xFreeColorsReq)) >> 2;
    pixels = (Pixel *) &req[1];

    FreePixels(client, req->cmap, num, pixels);
    return REQ_NOCHANGE;
}

static int
get_mod_map_req(client, data)
    ClientPtr   client;
    char       *data;
{
    ReplyStuffPtr nr;

    nr = NewReply(client);
    if (nr) {
	nr->sequenceNumber = LBXSequenceNumber(client);
	nr->request = X_LbxGetModifierMapping;
	nr->lbx_req = TRUE;
        nr->extension = client->server->lbxReq;

	SendGetModifierMapping(client);

	return REQ_REPLACE;
    } else
	return REQ_NOCHANGE;
}

void
FinishModmapReply(client, seqnum, kpm, data)
    ClientPtr   client;
    int         seqnum;
    int         kpm;
    pointer     data;
{
    xGetModifierMappingReply reply;
    int         len = kpm << 3;

    reply.type = X_Reply;
    reply.numKeyPerModifier = kpm;
    reply.sequenceNumber = seqnum;
    reply.length = len >> 2;

    WriteToClient(client, sizeof(xGetModifierMappingReply), &reply);
    WriteToClientPad(client, len, data);
}

static Bool
get_mod_map_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xGetModifierMappingReply reply;
    xLbxGetModifierMappingReply *rep;
    int         len;
    pointer     tag_data;
    ReplyStuffPtr nr;
    QueryTagRec qt;

    rep = (xLbxGetModifierMappingReply *) data;

    nr = GetReply(client);
    assert(nr);

    len = rep->keyspermod << 3;
    if (rep->tag) {
	if (rep->length) {	/* first time, set tag */

#ifdef LBX_STATS
	    getmodmap_full++;
#endif

	    tag_data = (pointer) &rep[1];
	    if (!TagStoreData(global_cache, rep->tag, len,
			      LbxTagTypeModmap, tag_data)) {
		/* tell server we lost it */
		SendInvalidateTag(client, rep->tag);
	    }
	} else {
	    tag_data = TagGetData(global_cache, rep->tag);
	    if (!tag_data) {
                qt.tag = rep->tag;
                qt.tagtype = LbxTagTypeModmap;
                qt.typedata.modmap.keyspermod = rep->keyspermod;
		/* lost data -- ask again for tag value */
		QueryTag(client, &qt);

		/* XXX what is the right way to stack Queries? */
		return TRUE;
	    }
#ifdef LBX_STATS
	    getmodmap_tag++;
            tag_bytes_unsent += (rep->keyspermod << 3);
#endif
	}
    } else {

#ifdef LBX_STATS
	getmodmap_full++;
#endif

	/* server didn't send us a tag for some reason -- just pass on data */
	tag_data = (pointer) &rep[1];
    }

    FinishModmapReply(client, rep->sequenceNumber, (int) rep->keyspermod,
			tag_data);

    return TRUE;
}

static int
get_key_map_req(client, data)
    ClientPtr   client;
    char       *data;
{
    ReplyStuffPtr nr;
    xGetKeyboardMappingReq *req;

    req = (xGetKeyboardMappingReq *) data;
    nr = NewReply(client);
    if (nr) {
	nr->sequenceNumber = LBXSequenceNumber(client);
	nr->request = X_LbxGetKeyboardMapping;
	nr->lbx_req = TRUE;
        nr->extension = client->server->lbxReq;
	nr->request_info.lbxgetkeymap.count = req->count;
	nr->request_info.lbxgetkeymap.first = req->firstKeyCode;

	SendGetKeyboardMapping(client);

	return REQ_REPLACE;
    } else
	return REQ_NOCHANGE;
}

/*
 * always ask for the whole map from server, and send requested subset to
 * client
 */
void
FinishKeymapReply(client, seqnum, kpk, first, count, data)
    ClientPtr   client;
    int         seqnum;
    int         kpk;
    int         first;
    int         count;
    char       *data;
{
    xGetKeyboardMappingReply reply;
    int         len = (kpk * count) << 2;

    reply.type = X_Reply;
    reply.keySymsPerKeyCode = kpk;
    reply.sequenceNumber = seqnum;
    reply.length = len >> 2;

    WriteToClient(client, sizeof(xGetKeyboardMappingReply), &reply);
    data += kpk * (first - LBXMinKeyCode(client));
    WriteToClientPad(client, len, data);
}


static Bool
get_key_map_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xGetKeyboardMappingReply reply;
    xLbxGetKeyboardMappingReply *rep;
    int         len;
    pointer     tag_data;
    ReplyStuffPtr nr;
    QueryTagRec	qt;

    rep = (xLbxGetKeyboardMappingReply *) data;

    nr = GetReply(client);
    assert(nr);

    if (rep->tag) {
	if (rep->length) {	/* first time, set tag */

#ifdef LBX_STATS
	    getkeymap_full++;
#endif

	    tag_data = (pointer) &rep[1];
	    len = rep->keysperkeycode *
		(LBXMaxKeyCode(client) - LBXMinKeyCode(client) + 1) * 4;
	    if (!TagStoreData(global_cache, rep->tag, len,
			      LbxTagTypeKeymap, tag_data)) {
		/* tell server we lost it */
		SendInvalidateTag(client, rep->tag);
	    }
	} else {
	    tag_data = TagGetData(global_cache, rep->tag);
	    if (!tag_data) {
		/* lost data -- ask again for tag value */

                qt.tag = rep->tag;
                qt.tagtype = LbxTagTypeKeymap;
                qt.typedata.keymap.keyspercode = rep->keysperkeycode;
                qt.typedata.keymap.count = nr->request_info.lbxgetkeymap.count;
                qt.typedata.keymap.first = nr->request_info.lbxgetkeymap.first;
		QueryTag(client, &qt);

		/* XXX what is the right way to stack Queries? */
		return TRUE;
	    }
#ifdef LBX_STATS
	    getkeymap_tag++;
            tag_bytes_unsent += (rep->keysperkeycode *
		(LBXMaxKeyCode(client) - LBXMinKeyCode(client) + 1) * 4);
#endif
	}
    } else {

#ifdef LBX_STATS
	getkeymap_full++;
#endif

	/* server didn't send us a tag for some reason -- just pass on data */
	tag_data = (pointer) &rep[1];
    }

    FinishKeymapReply(client, rep->sequenceNumber, (int) rep->keysperkeycode,
			nr->request_info.lbxgetkeymap.first,
			nr->request_info.lbxgetkeymap.count,
			tag_data);

    return TRUE;
}

static int
queryfont_req(client, data)
    ClientPtr   client;
    char       *data;
{
    ReplyStuffPtr nr;
    xResourceReq *req;

    req = (xResourceReq *) data;
    nr = NewReply(client);
    if (nr) {
	nr->sequenceNumber = LBXSequenceNumber(client);
	nr->request = X_LbxQueryFont;
	nr->lbx_req = TRUE;
        nr->extension = client->server->lbxReq;

	SendQueryFont(client, req->id);

	return REQ_REPLACE;
    } else
	return REQ_NOCHANGE;
}

void
FinishQueryFontReply(client, seqnum, length, data)
    ClientPtr   client;
    int         seqnum;
    int		length;
    pointer     data;
{
    xQueryFontReply *reply = (xQueryFontReply *)data;

    /* patch up certain fields */
    reply->type = X_Reply;
    reply->sequenceNumber = seqnum;
    reply->length = (length - sizeof(xGenericReply)) >> 2;

    length -= sizeof(xQueryFontReply);
    WriteToClient(client, sizeof(xQueryFontReply), reply);
    WriteToClient(client, length, reply + 1);
}


static Bool
get_queryfont_reply(client, data)
    ClientPtr   client;
    char       *data;
{
    xQueryFontReply reply;
    xLbxQueryFontReply *rep;
    int         len;
    pointer     tag_data;
    TagData	td;
    ReplyStuffPtr nr;
    QueryTagRec	qt;

    rep = (xLbxQueryFontReply *) data;

    nr = GetReply(client);
    assert(nr);

    if (rep->tag) {
	if (rep->length) {	/* first time, set tag */

#ifdef LBX_STATS
	    queryfont_full++;
#endif

	    tag_data = (pointer) &rep[1];
            len = rep->length << 2;
	    if (!TagStoreData(global_cache, rep->tag, len,
			      LbxTagTypeFont, tag_data)) {
		/* tell server we lost it */
		SendInvalidateTag(client, rep->tag);
	    }
	} else {
	    td = TagGetTag(global_cache, rep->tag);
	    if (!td) {
		/* lost data -- ask again for tag value */

                qt.tag = rep->tag;
                qt.tagtype = LbxTagTypeFont;
		QueryTag(client, &qt);

		/* XXX what is the right way to stack Queries? */
		return TRUE;
	    }
            len = td->size;
            tag_data = td->tdata;
#ifdef LBX_STATS
	    queryfont_tag++;
            tag_bytes_unsent += len;
#endif
	}
    } else {

#ifdef LBX_STATS
	queryfont_full++;
#endif

	/* server didn't send us a tag for some reason -- just pass on data */
	tag_data = (pointer) &rep[1];
        len = rep->length << 2;
    }

    FinishQueryFontReply(client, rep->sequenceNumber, len, tag_data);

    return TRUE;
}

int
ProcLBXInternAtom(client)
    ClientPtr   client;
{
    int         yank;

    yank = intern_atom_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXGetAtomName(client)
    ClientPtr   client;
{
    int         yank;

    yank = get_atom_name_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXCreateColormap(client)
    ClientPtr   client;
{
    int         yank;

    yank = create_colormap_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXAllocColor(client)
    ClientPtr   client;
{
    int         yank;

    yank = alloc_color_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXAllocNamedColor(client)
    ClientPtr   client;
{
    int         yank;

    yank = alloc_named_color_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXFreeColormap(client)
    ClientPtr   client;
{
    int         yank;

    yank = free_colormap_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXCopyColormapAndFree(client)
    ClientPtr   client;
{
    int         yank;

    yank = copy_colormap_and_free_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXFreeColors(client)
    ClientPtr   client;
{
    int         yank;

    yank = free_colors_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXLookupColor(client)
    ClientPtr   client;
{
    int         yank;

    yank = lookup_color_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXGetModifierMapping(client)
    ClientPtr   client;
{
    int         yank;

    yank = get_mod_map_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXGetKeyboardMapping(client)
    ClientPtr   client;
{
    int         yank;

    yank = get_key_map_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
ProcLBXQueryFont(client)
    ClientPtr   client;
{
    int         yank;

    yank = queryfont_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}

int
MakeLBXReply(client)
    ClientPtr   client;
{
    ReplyStuffPtr nr;

    REQUEST(xReq);

    /* create the reply struct for requests we don't do anything with */
    if (generates_replies(stuff->reqType)) {
	nr = NewReply(client);
	if (nr) {
	    nr->sequenceNumber = LBXSequenceNumber(client);
	    nr->request = stuff->reqType;
	    nr->extension = (stuff->reqType > X_NoOperation) ? stuff->reqType : 0;
	}
    }
    return FinishLBXRequest(client, REQ_PASSTHROUGH);
}

int
FinishLBXRequest(client, yank)
    ClientPtr   client;
    int         yank;
{
    REQUEST(xReq);

#define	yankable(y)	(((y) == REQ_YANK) || ((y) == REQ_REPLACE))

#ifdef PROTOCOL_SLOW
    LBXCacheSafe(client) = FALSE;
#endif

#ifdef PROTOCOL_FULL
    if (!yankable(yank) && (generates_events(stuff->reqType) ||
			    generates_replies(stuff->reqType)
			    || generates_errors(stuff->reqType)))
	LBXCacheSafe(client) = FALSE;
    else
	LBXCacheSafe(client) = TRUE;
#endif

#ifdef PROTOCOL_MOST
    if (!yankable(yank)
     (generates_events(stuff->reqType) || generates_replies(stuff->reqType)))
	LBXCacheSafe(client) = FALSE;
    else
	LBXCacheSafe(client) = TRUE;
#endif

#ifdef PROTOCOL_POOR
    LBXCacheSafe(client) = TRUE;
#endif

    if (yank == REQ_YANK) {
	LBXSequenceLost(client)++;
	LBXYanked(client)++;
	LBXLastReply(client) = LBXSequenceNumber(client);
	DBG(DBG_CLIENT, (stderr, "short-circuited client %d req %d\n",
			 client->index, stuff->reqType));
    }
    /* make sure server's sequence number is accurate */
    if ((!yankable(yank) && LBXSequenceLost(client)) ||
	    (LBXSequenceLost(client) >= MAX_SEQUENCE_LOST)) {
	BumpSequence(client);
    }
    if (yank == REQ_NOCHANGE) {
	WriteReqToServer(client, stuff->length << 2, stuff);
    }
    return Success;
}

/*
 * need to rewrite error codes for requests we've replaced.
 *
 * QueryFont regularly hits this in normal operation
 */
static void
patchup_error(err, nr)
    xError     *err;
    ReplyStuffPtr nr;
{
    QueryTagPtr qtp;

    switch (err->minorCode) {
    case X_LbxGetModifierMapping:
	err->minorCode = X_GetModifierMapping;
	break;
    case X_LbxGetKeyboardMapping:
	err->minorCode = X_GetKeyboardMapping;
	break;
    case X_LbxGetProperty:
	err->minorCode = X_GetProperty;
	break;
    case X_LbxQueryFont:
	err->minorCode = X_QueryFont;
	break;
    case X_LbxQueryTag:
	qtp = &(nr->request_info.lbxquerytag.info);
	switch (qtp->tagtype) {
	case LbxTagTypeModmap:
	    err->minorCode = X_GetModifierMapping;
	    break;
	case LbxTagTypeKeymap:
	    err->minorCode = X_GetKeyboardMapping;
	    break;
	case LbxTagTypeProperty:
	    err->minorCode = X_GetProperty;
	    break;
	case LbxTagTypeFont:
	    err->minorCode = X_QueryFont;
	    break;
	default:
	    assert(0);
	    break;
	}
	break;
    default:
	assert(0);
	break;
    }
}

static Bool
error_matches(client,nr, err)
    ClientPtr	client;
    ReplyStuffPtr nr;
    xError     *err;
{
    if (nr && (err->sequenceNumber == (nr->sequenceNumber & 0xffff))) {
	if (err->majorCode == client->server->lbxReq &&
		(err->minorCode == nr->request)) {
	    return TRUE;
	} else if (err->majorCode == nr->request) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * NOTE
 *
#ifdef notyet
 * data coming in may not be on a word boundary, so it has to be
 * shuffled up
#endif
 *
 * returns TRUE if data (possibly modified) is to be sent on to client,
 * FALSE if data is replaced
 */
Bool
DoLBXReply(client, data, len)
    ClientPtr   client;
    char       *data;
    int         len;
{
    xGenericReply *reply;
    int         rlen;
    xError     *err;
    ReplyStuffPtr nr;
    Bool        remove_it = TRUE;
    Bool        ret = TRUE;

#ifdef PROTOCOL_RAW
    return;
#endif

    /* make sure data is on word boundary */

#ifdef notyet
    bcopy(data, temp_buffer, len);
    reply = (xGenericReply *) temp_buffer;
#else
    reply = (xGenericReply *) data;
#endif

    if (client->awaitingSetup) {/* snarf setup info */
	GetConnectionInfo(client, reply, len);
	return TRUE;
    }

#ifdef old
    rlen = reply_length(client, reply);

    if (len != rlen)
	fprintf(stderr, "HELP -- reply length not the same as buffer length\n");
#endif

#ifdef DEBUG
    if (reply->sequenceNumber < LBXLastReply(client))
	fprintf(stderr, "replies out of whack for client %d\n", client->index);
#endif

    if (reply->sequenceNumber > LBXSequenceNumber(client))
	fprintf(stderr, "BOGUS DATA\n");

    if (reply->type != X_Reply) {	/* event or error */

#ifdef TRACE
	fprintf(stderr, "got event/error %d (%d %d)\n",
		(reply->type & 0x7f), reply->sequenceNumber, client->XRemId);
#endif

	if ((reply->type & 0x7f) >= LASTEvent)
	    fprintf(stderr, "Extension event or BOGUS REPLY TYPE\n");

	/* clear out pending replies that resulted in errors */
	if (reply->type == X_Error) {
	    err = (xError *) reply;
	    nr = GetReply(client);
            if (error_matches(client, nr, err)) {
                if (err->majorCode == client->server->lbxReq)
		    patchup_error(err, nr);
		RemoveReply(client);
	    }
	}
	/* KeymapNotify has no sequence # */
	if ((reply->sequenceNumber < LBXLastReply(client)) &&
		reply->type != KeymapNotify) {
	    /*
	     * the server's sequence number is out-of-date. fix the
	     * event/error so its correct
	     */
	    err = (xError *) reply;

#ifdef DEBUG
	    fprintf(stderr, "rewriting error/event sequence number from %d to %d\n",
		    err->sequenceNumber, client->sequenceNumber);
#endif

	    err->sequenceNumber = LBXLastReply(client);

#ifdef notyet
	    /* move the changed value into the proper place */
	    bcopy(temp_buffer, data, len);
#endif
	}
	if (reply->type == MotionNotify) {
	    AllowMotion(client, 1);
	}
	return TRUE;
    }
    nr = GetReply(client);

    if (!nr)			/* what is this???? */
	return TRUE;

    /*
     * all unknown (extension) requests get a reply struct.  since we don't
     * know if there's really a reply for them or not, we can't clean them out
     * till we do get a reply.  at this point, we clear them out till we reach
     * either a safe reply struct, or an extension struct that has a matching
     * sequence number.
     */
    while (!nr->lbx_req && nr->extension && NumReplies(client) > 1) {
	if ((nr->sequenceNumber & 0xffff) == reply->sequenceNumber)
	    break;
	RemoveReply(client);
	nr = GetReply(client);
	assert(nr);
    }


#ifdef TRACE
    fprintf(stderr, "got reply supposedly for %d (%d %d)\n",
	    nr->request, reply->sequenceNumber, client->index);
#endif

    /* sequence number coming back should be the same as what's coming out */
    if ((nr->sequenceNumber & 0xffff) != reply->sequenceNumber)
	fprintf(stderr, "HELP -- reply for %d out-of-sync %d %d\n",
		nr->request,
		reply->sequenceNumber, nr->sequenceNumber);

#ifndef PROTOCOL_SLOW
    LBXCacheSafe(client) = TRUE;/* found matching reply, go for it */
#endif

    if (!nr->extension) {
	switch (nr->request) {
	case X_InternAtom:
	    remove_it = intern_atom_reply(client, reply);
	    break;
	case X_GetAtomName:
	    remove_it = get_atom_name_reply(client, reply);
	    break;
	case X_LookupColor:
	    remove_it = lookup_color_reply(client, reply);
	    break;
	case X_AllocColor:
	    remove_it = alloc_color_reply(client, reply);
	    break;
	case X_AllocNamedColor:
	    remove_it = alloc_named_color_reply(client, reply);
	    break;
	default:
	    break;
	}
    } else if (nr->lbx_req) {
        /* handle various extensions we know about */
	switch (nr->request) {
	case X_LbxGetModifierMapping:
	    remove_it = get_mod_map_reply(client, reply);
	    ret = FALSE;
	    break;
	case X_LbxGetKeyboardMapping:
	    remove_it = get_key_map_reply(client, reply);
	    ret = FALSE;
	    break;
	case X_LbxGetProperty:
	    remove_it = GetLbxGetPropertyReply(client, reply);
	    ret = FALSE;
	    break;
	case X_LbxQueryFont:
	    remove_it = get_queryfont_reply(client, reply);
	    ret = FALSE;
	    break;
	case X_LbxQueryTag:
	    remove_it = GetQueryTagReply(client, reply);
	    ret = FALSE;
	    break;
	default:
	    break;
	}
    } else {
    	/* XXX handle any other extensions we may know about */
    }
    if (remove_it)
	RemoveReply(client);
    return ret;
}
