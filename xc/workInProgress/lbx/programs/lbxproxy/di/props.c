/* $XConsortium: XIE.h,v 1.3 94/01/12 19:36:23 rws Exp $ */
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
 * $NCDId: @(#)props.c,v 1.2 1994/02/11 00:14:25 lemke Exp $
 */
/*
 * property handling
 */


#include	<stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include	<X11/Xproto.h>
#include	"assert.h"
#include	"lbxdata.h"
#include	"util.h"
#include	"tags.h"
#include	"lbx.h"		/* gets dixstruct.h */
#include	"resource.h"
#include	"wire.h"
#define _XLBX_SERVER_
#include	"lbxstr.h"	/* gets dixstruct.h */

/*
 * XXX
 *
 * GetProperty notes
 *
 * GetProperty is handled by using X_LbxGetProperty to get the
 * entire property.  if the client asked for a subset, we only
 * ship that subset
 *
 * XXX is this a good idea?  will we transfer data that never gets
 * used?  or is it so useful that offset & length fields aren't
 * required in X_LbxGetProperty?
 */


/*
 * lots of brains need to live here.  this decides whether or not
 * to use an LbxChangeProperty
 *
 * heuristics:
 *
 *	- don't rewrite if its 'small' -- roundtrip swamps bandwidth
 *	- don't rewrite if its a WM property and the WM isn't sharing
 *	  the proxy
 *	- don't rewrite if the prop is on a window owned by a non-proxy
 *	  client
 *
 *	- do rewrite if its a prop on one of the client's windows
 *	- do rewrite if its a Selection property (hope is that
 *	  receiver is also using proxy)
 */
static Bool
rewrite_change_prop()
{
}

static int
get_property_req(client, data)
    ClientPtr   client;
    char       *data;
{
    ReplyStuffPtr nr;
    xGetPropertyReq *req;

    req = (xGetPropertyReq *) data;
    nr = NewReply(client);
    if (nr) {
	nr->sequenceNumber = LBXSequenceNumber(client);
	nr->request = X_LbxGetProperty;
	nr->lbx_req = TRUE;
        nr->extension = client->server->lbxReq;
        nr->request_info.lbxgetprop.offset = req->longOffset;
        nr->request_info.lbxgetprop.length = req->longLength;

        /* note we ask for the whole damn thing -- any truncating is
         * done when we pass it on
         */
	SendGetProperty(client, req->window, req->property, req->type, 
        	req->delete, 0, 1000000);

	return REQ_REPLACE;
    } else
	return REQ_NOCHANGE;
}

void
FinishGetPropertyReply(client, seqnum, offset, length, ptdp, pdata)
    ClientPtr   client;
    int         seqnum;
    CARD32		offset;
    CARD32		length;
    PropertyTagDataPtr	ptdp;
    char               *pdata;
{
    xGetPropertyReply reply;
    int         bytesafter,
                t,
                dlen;

    t = ptdp->length - (4 * offset);
    dlen = min(t, (4 * length));
    bytesafter = ptdp->length - (dlen + 4 * offset);

    if (!pdata)
    	pdata = ptdp->data;

    pdata = pdata + (4 * offset);

    reply.type = X_Reply;
    reply.sequenceNumber = seqnum;
    reply.format = ptdp->format;
    reply.length = (dlen + 3) >> 2;;
    reply.propertyType = ptdp->type;
    reply.bytesAfter = bytesafter;
    if (dlen && ptdp->format)	/* avoid div-by-zero if no data */
	reply.nItems = dlen / (ptdp->format / 8);
    else
	reply.nItems = 0;

    WriteToClient(client, sizeof(xGetPropertyReply), (char *) &reply);
    if (dlen)
	WriteToClient(client, dlen, pdata);
}

Bool
GetLbxGetPropertyReply(client, data)
    ClientPtr   client;
    char       *data;
{
    xGetPropertyReply reply;
    xLbxGetPropertyReply *rep;
    int         len;
    pointer     pdata;
    ReplyStuffPtr nr;
    PropertyTagDataRec ptd;
    PropertyTagDataPtr ptdp;
    QueryTagRec qt;

    rep = (xLbxGetPropertyReply *) data;

    nr = GetReply(client);
    assert(nr);

    ptd.type = rep->propertyType;
    ptd.format = rep->format;
    ptd.data = NULL;
    len = rep->length << 2;
    ptd.length = len;
    ptdp = &ptd;
    if (rep->tag) {
	if (rep->length) {	/* first time, set tag */

#ifdef LBX_STATS
	    getprop_full++;
#endif

	    pdata = (pointer) xalloc(len);
	    if (!pdata) {
		xfree(pdata);
		/* tell server we lost it */
		SendInvalidateTag(client, rep->tag);
		ptd.data = (pointer) &rep[1];
		FinishGetPropertyReply(client, client->sequence,
				       nr->request_info.lbxgetprop.offset,
				    nr->request_info.lbxgetprop.length, &ptd, NULL);
		return TRUE;
	    }
	    bcopy((char *) &rep[1], (char *) pdata, len);
	    ptd.data = pdata;
	    if (!TagStoreData(prop_cache, rep->tag,
			      len + sizeof(PropertyTagDataRec),
			      LbxTagTypeProperty, (pointer) &ptd, NULL)) {
		/* tell server we lost it */
		SendInvalidateTag(client, rep->tag);
	    }
	} else {
	    ptdp = (PropertyTagDataPtr) TagGetData(prop_cache, rep->tag);
	    if (!ptdp) {
		/* lost data -- ask again for tag value */

		qt.tag = rep->tag;
		qt.tagtype = LbxTagTypeProperty;
		qt.typedata.getprop.offset = nr->request_info.lbxgetprop.offset;
		qt.typedata.getprop.length = nr->request_info.lbxgetprop.length;
                qt.typedata.getprop.ptd = ptd;
		QueryTag(client, &qt);

		/* XXX what is the right way to stack Queries? */
		return TRUE;
	    }
#ifdef LBX_STATS
	    getprop_tag++;
            tag_bytes_unsent += ptdp->length;
#endif
	}
    } else {

#ifdef LBX_STATS
	getprop_full++;
#endif

	/* server didn't send us a tag -- either can't store it or no 
         * prop data
         */
        if (rep->length)
	    ptd.data = (pointer) &rep[1];
    }

    FinishGetPropertyReply(client, rep->sequenceNumber,
		      nr->request_info.lbxgetprop.offset,
		      nr->request_info.lbxgetprop.length,
		      ptdp, NULL);

    return TRUE;
}

int
ProcLBXGetProperty(client)
    ClientPtr	client;
{
    int         yank;
    REQUEST(xGetPropertyReq);

    /* just pass through 'delete' messages */
    if (stuff->delete) {
#ifdef DEBUG
	fprintf(stderr, "Got GetProperty with 'delete' set -- passing through\n");
#endif
    	return ProcStandardRequest(client);
    }

    yank = get_property_req(client, client->requestBuffer);
    return FinishLBXRequest(client, yank);
}
