/* $XConsortium: gfx.c,v 1.11 94/11/22 22:26:51 mor Exp mor $ */
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
 * $NCDId: @(#)gfx.c,v 1.3 1994/03/24 17:54:44 lemke Exp $
 */
/*
 * graphics acceleration
 */

#include	<stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include	<X11/X.h>	/* for KeymapNotify */
#include	<X11/Xproto.h>
#include	"assert.h"
#include	"lbxdata.h"
#include	"util.h"
#include	"lbx.h"		/* gets dixstruct.h */
#include	"resource.h"
#include	"wire.h"
#define _XLBX_SERVER_
#include	"lbxstr.h"	/* gets dixstruct.h */
#include        "lbximage.h"

static int  pad[4] = {0, 3, 2, 1};

/*
 * Routines for re-encoding line, rectangle and arc requests
 */

#define REENCODE_SHORT(val, out) \
    if ((val) >= (short)0x0800 || (val) < (short)0xf800) \
	return 0; \
    if ((val) <= -113 || (val) >= 128) \
	*(out)++ = 0x80 | (((val) >> 8) & 0x0f); \
    *(out)++ = (val) & 0xff;

#define REENCODE_USHORT(val, out) \
    if ((unsigned short)(val) >= 0x1000) \
	return 0; \
    if ((val) >= 240) \
	*(out)++ = 0xf0 | ((val) >> 8); \
    *(out)++ = (val) & 0xff;

#define REENCODE_ANGLE(val, out) \
    if ((val) < ((-360) << 6) || (val) > (360 << 6)) \
	return 0; \
    if (((val) & 0x007f) == 0) { \
	short angle = (val) >> 7; \
	if (angle % 45) { \
	    *(out)++ = (val) >> 8; \
	    *(out)++ = (val) & 0xff; \
	} \
	else \
	    *(out)++ = 0x80 | ((angle / 45) + 4); \
    }

static int
reencode_poly(client, lbxreq, reencode_rtn)
    ClientPtr   client;
    CARD8       lbxreq;
    int         (*reencode_rtn) ();
{
    REQUEST(xPolyPointReq);
    XServerPtr  server = client->server;
    int         len = stuff->length << 2;
    xLbxPolyPointReq *newreq = NULL;
    int         bytes;

    /* If it's delta compressible, don't try to reencode */
    if (DELTA_CACHEABLE(&server->outdeltas, len) ||
	    (newreq = (xLbxPolyPointReq *) xalloc(len)) == NULL ||
	    (bytes = (*reencode_rtn) (client, (short *) &stuff[1],
				      (char *) newreq + sz_xLbxPolyPointReq,
				      (len - sz_xPolyPointReq) >> 2)) == 0) {
	if (newreq)
	    xfree(newreq);
	return ProcStandardRequest(client);
    }
    FinishLBXRequest(client, REQ_PASSTHROUGH);

    newreq->reqType = server->lbxReq;
    newreq->lbxReqType = lbxreq;
    newreq->drawable = stuff->drawable;
    newreq->gc = stuff->gc;
    len = sz_xLbxPolyPointReq + bytes;
    newreq->padBytes = pad[len % 4];
    len += newreq->padBytes;
    newreq->length = len >> 2;
    WriteToServer(client, len, (char *) newreq);
    xfree(newreq);
    return Success;
}

/* ARGSUSED */
static int
reencode_points_origin(client, in, out, count)
    ClientPtr   client;
    register short *in;
    register char *out;
    int         count;
{
    register short diff;
    int         i;
    short       last_x = 0;
    short       last_y = 0;
    char       *start_out = out;

    for (i = 0; i < count; i++, in += 2) {
	diff = in[0] - last_x;
	last_x = in[0];
	REENCODE_SHORT(diff, out);
	diff = in[1] - last_y;
	last_y = in[1];
	REENCODE_SHORT(diff, out);
    }
    return out - start_out;
}

/* ARGSUSED */
static int
reencode_points_previous(client, in, out, count)
    ClientPtr   client;
    register short *in;
    register char *out;
    int         count;
{
    int         i;
    char       *start_out = out;

    for (i = 0; i < count; i++, in += 2) {
	REENCODE_SHORT(in[0], out);
	REENCODE_SHORT(in[1], out);
    }
    return out - start_out;
}

/* ARGSUSED */
static int
reencode_segment(client, in, out, count)
    ClientPtr   client;
    register short *in;
    register char *out;
    int         count;
{
    register short diff;
    int         i;
    short       last_x = 0;
    short       last_y = 0;
    char       *start_out = out;

    count >>= 1;
    for (i = 0; i < count; i++, in += 4) {
	/* reencode (x1, y1) */
	diff = in[0] - last_x;
	last_x = in[0];
	REENCODE_SHORT(diff, out);
	diff = in[1] - last_y;
	last_y = in[1];
	REENCODE_SHORT(diff, out);

	/* reencode (x2, y2) */
	diff = in[2] - last_x;
	REENCODE_SHORT(diff, out);
	diff = in[3] - last_y;
	REENCODE_SHORT(diff, out);
    }
    return out - start_out;
}

/* ARGSUSED */
static int
reencode_rectangle(client, in, out, count)
    ClientPtr   client;
    register short *in;
    register char *out;
    int         count;
{
    register short diff;
    int         i;
    short       last_x = 0;
    short       last_y = 0;
    char       *start_out = out;

    count >>= 1;
    for (i = 0; i < count; i++, in += 4) {
	/* reencode (x, y) origin */
	diff = in[0] - last_x;
	last_x = in[0];
	REENCODE_SHORT(diff, out);
	diff = in[1] - last_y;
	last_y = in[1];
	REENCODE_SHORT(diff, out);

	/* reencode (width, height) */
	REENCODE_USHORT(in[2], out);
	REENCODE_USHORT(in[3], out);
    }
    return out - start_out;
}

/* ARGSUSED */
static int
reencode_arc(client, in, out, count)
    ClientPtr   client;
    register short *in;
    register char *out;
    int         count;
{
    register short diff;
    int         i;
    short       last_x = 0;
    short       last_y = 0;
    char       *start_out = out;

    count = count / 3;
    for (i = 0; i < count; i++, in += 6) {
	/* reencode (x, y) origin */
	diff = in[0] - last_x;
	last_x = in[0];
	REENCODE_SHORT(diff, out);
	diff = in[1] - last_y;
	last_y = in[1];
	REENCODE_SHORT(diff, out);

	/* reencode (width, height) */
	REENCODE_USHORT(in[2], out);
	REENCODE_USHORT(in[3], out);

	/* reencode (angle1, angle2) */
	REENCODE_ANGLE(in[4], out);
	REENCODE_ANGLE(in[5], out);
    }
    return out - start_out;
}

int
ProcLBXPolyPoint(client)
    ClientPtr   client;
{
    REQUEST(xPolyPointReq);
    int         (*reencode_rtn) ();

    reencode_rtn = (stuff->coordMode) ? reencode_points_previous :
	reencode_points_origin;
    return reencode_poly(client, X_LbxPolyPoint, reencode_rtn);
}

int
ProcLBXPolyLine(client)
    ClientPtr   client;
{
    REQUEST(xPolyLineReq);
    int         (*reencode_rtn) ();

    reencode_rtn = (stuff->coordMode) ? reencode_points_previous :
	reencode_points_origin;
    return reencode_poly(client, X_LbxPolyLine, reencode_rtn);
}

int
ProcLBXPolySegment(client)
    ClientPtr   client;
{
    return reencode_poly(client, X_LbxPolySegment, reencode_segment);
}

int
ProcLBXPolyRectangle(client)
    ClientPtr   client;
{
    return reencode_poly(client, X_LbxPolyRectangle, reencode_rectangle);
}

int
ProcLBXPolyArc(client)
    ClientPtr   client;
{
    return reencode_poly(client, X_LbxPolyArc, reencode_arc);
}

int
ProcLBXFillPoly(client)
    ClientPtr   client;
{
    REQUEST(xFillPolyReq);
    XServerPtr  server = client->server;
    int         len = stuff->length << 2;
    xLbxFillPolyReq *newreq = NULL;
    int         bytes;
    int         (*reencode_rtn) ();

    reencode_rtn = (stuff->coordMode) ? reencode_points_previous :
	reencode_points_origin;

    /* If it's delta compressible, don't try to reencode */
    if (DELTA_CACHEABLE(&server->outdeltas, len) ||
	    (newreq = (xLbxFillPolyReq *) xalloc(len)) == NULL ||
	    (bytes = (*reencode_rtn) (client, (short *) &stuff[1],
				      (char *) newreq + sz_xLbxFillPolyReq,
				      (len - sz_xFillPolyReq) >> 2)) == 0) {
	if (newreq)
	    xfree(newreq);
	return ProcStandardRequest(client);
    }
    FinishLBXRequest(client, REQ_PASSTHROUGH);

    newreq->reqType = server->lbxReq;
    newreq->lbxReqType = X_LbxFillPoly;
    newreq->drawable = stuff->drawable;
    newreq->gc = stuff->gc;
    newreq->shape = stuff->shape;
    len = sz_xLbxPolyPointReq + bytes;
    newreq->padBytes = pad[len % 4];
    len += newreq->padBytes;
    newreq->length = len >> 2;
    WriteToServer(client, len, (char *) newreq);
    xfree(newreq);
    return Success;
}

int
ProcLBXPolyFillRectangle(client)
    ClientPtr   client;
{
    return reencode_poly(client, X_LbxPolyFillRectangle, reencode_rectangle);
}

int
ProcLBXPolyFillArc(client)
    ClientPtr   client;
{
    return reencode_poly(client, X_LbxPolyFillArc, reencode_arc);
}


static void
SwapXPutImage (req)

xPutImageReq *req;

{
    char n;

    swaps (&req->length, n);
    swapl (&req->drawable, n);
    swapl (&req->gc, n);
    swaps (&req->width, n);
    swaps (&req->height, n);
    swaps (&req->dstX, n);
    swaps (&req->dstY, n);
}


int
ProcLBXPutImage (client)

ClientPtr   client;

{
    REQUEST		(xPutImageReq);
    XServerPtr  	server = client->server;
    xLbxPutImageReq	*newreq = NULL;
    int         	len, method, compBytes, status;
    float		percentCompression;

    if (client->swapped)
	SwapXPutImage (stuff);

    len = stuff->length << 2;

    if (!((stuff->format == ZPixmap && stuff->depth == 8) ||
	   stuff->depth == 1))
    {
	status = LBX_IMAGE_COMPRESS_UNSUPPORTED_FORMAT;
    }
    else if ((newreq = (xLbxPutImageReq *) xalloc (len)) == NULL)
    {
	status = LBX_IMAGE_COMPRESS_BAD_MALLOC;
    }
    else
    {
	if (stuff->depth == 1)
	{
	    status = LbxImageEncodeFaxG42D ((unsigned char *) &stuff[1],
		      (unsigned char *) newreq + sz_xLbxPutImageReq,
		      len - sz_xLbxPutImageReq,
		      len - sz_xPutImageReq,
		      (int) stuff->width,
		      LBXBitmapBitOrder(client) == LSBFirst,
		      &compBytes);

	    method = LbxImageCompressFaxG42D;
	}
	else /* depth 8 and ZPixmap format */
	{
	    status = LbxImageEncodePackBits ((char *) &stuff[1],
		      (char *) newreq + sz_xLbxPutImageReq,
		      len - sz_xLbxPutImageReq,
		      (int) stuff->height, (int) stuff->width, &compBytes);

	    method = LbxImageCompressPackBits;
	}
    }

#ifdef LBX_STATS
	fprintf (stderr,
	    "PutImage (xid=%x, w=%d, h=%d, x=%d, y=%d): ",
	    stuff->drawable, stuff->width, stuff->height,
	    stuff->dstX, stuff->dstY);
#endif

    if (status != LBX_IMAGE_COMPRESS_SUCCESS)
    {
#ifdef LBX_STATS
	fprintf (stderr, "image not compressed - ");

	if (status == LBX_IMAGE_COMPRESS_UNSUPPORTED_FORMAT)
	    fprintf (stderr, "unsupported format\n");
	else if (status == LBX_IMAGE_COMPRESS_NOT_WORTH_IT)
	    fprintf (stderr, "not worth it\n");
	else if (status == LBX_IMAGE_COMPRESS_BAD_MALLOC)
	    fprintf (stderr, "bad malloc\n");
#endif
	if (newreq)
	    xfree (newreq);

	if (client->swapped)   /* Swap it back */
	    SwapXPutImage (stuff);

	return ProcStandardRequest (client);
    }

    if (client->swapped)
    {
	/*
	 * Swap the length back because FinishLBXRequest will swap it.
	 */

	char n;
	swaps (&stuff->length, n);
    }

    FinishLBXRequest (client, REQ_PASSTHROUGH);

    newreq->reqType = server->lbxReq;
    newreq->lbxReqType = X_LbxPutImage;
    newreq->compressionMethod = method;
    newreq->drawable = stuff->drawable;
    newreq->gc = stuff->gc;
    newreq->width = stuff->width;
    newreq->height = stuff->height;
    newreq->dstX = stuff->dstX;
    newreq->dstY = stuff->dstY;
    newreq->format = stuff->format;
    newreq->leftPad = stuff->leftPad;
    newreq->depth = stuff->depth;

    len = sz_xLbxPutImageReq + compBytes;
    newreq->padBytes = pad[len % 4];
    len += newreq->padBytes;
    newreq->lbxLength = len >> 2;
    newreq->xLength = stuff->length;
    
#ifdef LBX_STATS
    percentCompression = 100.0 * (1.0 -
	((float) (compBytes + pad[compBytes % 4])/
	 (float) ((stuff->length << 2) - sz_xPutImageReq)));

    fprintf (stderr, "%f percent compression ", percentCompression);
    if (method == LbxImageCompressFaxG42D)
	fprintf (stderr, "(FAX G42D)\n");
    else if (method == LbxImageCompressPackBits)
	fprintf (stderr, "(PackBits)\n");
#endif

    if (client->swapped)
	SwapPutImage (newreq);

    UncompressWriteToServer (client, len, (char *) newreq);

    xfree (newreq);
    return Success;
}


static int
get_image_req (client, data)

ClientPtr   client;
char       *data;

{
    ReplyStuffPtr nr;
    xGetImageReq *req;

    req = (xGetImageReq *) data;
    nr = NewReply (client);

    if (nr)
    {
	if (client->swapped)
	{
	    char n;

	    swapl (&req->drawable, n);
	    swaps (&req->x, n);
	    swaps (&req->y, n);
	    swaps (&req->width, n);
	    swaps (&req->height, n);
	    swapl (&req->planeMask, n);
	}

	nr->sequenceNumber = LBXSequenceNumber (client);
	nr->request = X_LbxGetImage;
	nr->lbx_req = TRUE;
	nr->extension = client->server->lbxReq;
	nr->request_info.lbxgetimage.width = req->width;
	nr->request_info.lbxgetimage.height = req->height;
#ifdef LBX_STATS
	nr->request_info.lbxgetimage.drawable = req->drawable;
	nr->request_info.lbxgetimage.x = req->x;
	nr->request_info.lbxgetimage.y = req->y;
#endif
	/*
	 * this expects a reply.  since we write the data here, we have to be
	 * sure the seq number is in sync first
	 */

	ForceSequenceUpdate (client);

	SendGetImage (client, req->drawable, req->x, req->y,
	    req->width, req->height, req->planeMask, req->format);

	return REQ_REPLACE;
    }
    else
	return REQ_NOCHANGE;
}

int
ProcLBXGetImage (client)

ClientPtr client;

{
    int yank;

    yank = get_image_req (client, client->requestBuffer);
    return FinishLBXRequest (client, yank);
}


Bool
GetLbxImageReply (client, data)

ClientPtr   client;
char       *data;

{
    xLbxGetImageReply	*rep;
    xGetImageReply	reply;
    int         	len;
    pointer     	imageData;
    ReplyStuffPtr	nr;
    int			freeIt = 1;

    rep = (xLbxGetImageReply *) data;

    nr = GetReply (client);
    assert (nr);

    if (client->swapped)
    {
	char n;
	swapl (&rep->xLength, n);
	swapl (&rep->visual, n);
    }

    if ((imageData = (pointer) xalloc (rep->xLength << 2)) == NULL)
	return FALSE;

    if (rep->compressionMethod == LbxImageCompressFaxG42D)
    {
	LbxImageDecodeFaxG42D (
	    (unsigned char *) &rep[1], (unsigned char *) imageData,
	    (int) ((rep->xLength << 2) - sz_xPutImageReq),
	    (int) nr->request_info.lbxgetimage.width,
	    LBXBitmapBitOrder(client) == LSBFirst);
    }
    else if (rep->compressionMethod == LbxImageCompressPackBits)
    {
	LbxImageDecodePackBits ((char *) &rep[1], (char *) imageData,
	    (int) nr->request_info.lbxgetimage.height,
	    (int) nr->request_info.lbxgetimage.width);
    }
    else if (rep->compressionMethod == LbxImageCompressNone)
    {
	xfree ((char *) imageData);
	imageData = (pointer) &rep[1];
	freeIt = 0;
    }

    reply.type = X_Reply;
    reply.depth = rep->depth;
    reply.sequenceNumber = rep->sequenceNumber;
    reply.length = rep->xLength;
    reply.visual = rep->visual;

    if (client->swapped)
	SwapGetImageReply (&reply);

#ifdef LBX_STATS
    fprintf (stderr, "GetImage (xid=%x, w=%d, h=%d, x=%d, y=%d): ",
	    nr->request_info.lbxgetimage.drawable,
	    nr->request_info.lbxgetimage.width,
	    nr->request_info.lbxgetimage.height,
	    nr->request_info.lbxgetimage.x,
	    nr->request_info.lbxgetimage.y);

    if (rep->compressionMethod == LbxImageCompressNone)
    {
	fprintf (stderr, "image not compressed\n");
    }
    else
    {
	float percentCompression = 100.0 * (1.0 -
	    ((float) rep->lbxLength / (float) rep->xLength));

	fprintf (stderr, "%f percent compression ", percentCompression);
	if (rep->compressionMethod == LbxImageCompressFaxG42D)
	    fprintf (stderr, "(FAX G42D)\n");
	else if (rep->compressionMethod == LbxImageCompressPackBits)
	    fprintf (stderr, "(PackBits)\n");
    }
#endif

    WriteToClient (client, sizeof (xGetImageReply), &reply);
    WriteToClientPad (client, rep->xLength << 2, imageData);

    if (freeIt)
	xfree ((char *) imageData);

    return TRUE;
}
