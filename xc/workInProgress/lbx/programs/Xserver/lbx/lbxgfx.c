/* $XConsortium: lbxgfx.c,v 1.8 94/11/09 21:21:23 mor Exp mor $ */
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
 * $NCDId: @(#)lbxgfx.c,v 1.4 1994/03/24 17:54:34 lemke Exp $
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
#include "windowstr.h"
#include "scrnintstr.h"
#define _XLBX_SERVER_
#include "lbxstr.h"
#include "lbxserve.h"
#include "lbxtags.h"
#include "Xfuncproto.h"
#include "lbximage.h"

extern int (*ProcVector[256])();

int
LbxDecodePoly(client, xreqtype, decode_rtn)
    register ClientPtr  client;
    CARD8		xreqtype;
    int			(*decode_rtn)();
{
    REQUEST(xLbxPolyPointReq);
    char		*in;
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
    client->requestBuffer = (pointer)xreq;

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
    xFillPolyReq	*xreq;
    int			len;
    int			retval;

    len = (stuff->length << 2) - sz_xLbxFillPolyReq;
    if ((xreq = (xFillPolyReq *) 
	    xalloc(sizeof(xFillPolyReq) + (len << 1))) == NULL)
	return BadAlloc;
    in = (char *)stuff + sz_xLbxFillPolyReq;
    len = LbxDecodePoints(in, in + len - stuff->padBytes, (short *) &xreq[1]);
    xreq->reqType = X_FillPoly;
    xreq->drawable = stuff->drawable;
    xreq->gc = stuff->gc;
    xreq->shape = stuff->shape;
    xreq->coordMode = 1;
    client->req_len = xreq->length = (sizeof(xFillPolyReq) + len) >> 2;
    client->requestBuffer = (pointer)xreq;

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


int
LbxDecodePutImage (client)

register ClientPtr  client;

{
    REQUEST		(xLbxPutImageReq);
    char		*in;
    xPutImageReq	*xreq;
    int			len;
    int			retval;

    if (stuff->compressionMethod != LbxImageCompressFaxG42D &&
        stuff->compressionMethod != LbxImageCompressPackBits)
    {
	return BadValue;
    }

    if ((xreq = (xPutImageReq *) xalloc (stuff->xLength << 2)) == NULL)
	return BadAlloc;

    in = (char *) stuff + sz_xLbxPutImageReq;

    if (stuff->compressionMethod == LbxImageCompressFaxG42D)
    {
	len = LbxImageDecodeFaxG42D (
	    (unsigned char *) in, (unsigned char *) &xreq[1],
	    (int) ((stuff->xLength << 2) - sz_xPutImageReq),
	    (int) stuff->width);
    }
    else
    {
	len = LbxImageDecodePackBits (in, (char *) &xreq[1],
	    (int) stuff->height, (int) stuff->width);
    }

    xreq->reqType = X_PutImage;
    xreq->drawable = stuff->drawable;
    xreq->gc = stuff->gc;
    xreq->width = stuff->width;
    xreq->height = stuff->height;
    xreq->format = stuff->format;
    xreq->dstX = stuff->dstX;
    xreq->dstY = stuff->dstY;
    xreq->leftPad = stuff->leftPad;
    xreq->depth = stuff->depth;

    client->req_len = xreq->length = stuff->xLength;
    client->requestBuffer = (pointer) xreq;

    retval = (*ProcVector[X_PutImage])(client);
    xfree(xreq);
    return retval;
}



int
GetTheImage (client, stuff, depth, visual, theImage, imageBytes)

register ClientPtr client;
xLbxGetImageReq *stuff;
int *depth;
VisualID *visual;
char **theImage;
int *imageBytes;

{
    register DrawablePtr pDraw;
    int			lines, linesPerBuf;
    register int	height;
    long		widthBytesLine, length;
    Mask		plane;
    char		*pBuf;

    height = stuff->height;
    if ((stuff->format != XYPixmap) && (stuff->format != ZPixmap))
    {
	client->errorValue = stuff->format;
        return(BadValue);
    }

    VERIFY_DRAWABLE(pDraw, stuff->drawable, client);

    *depth = pDraw->depth;

    if(pDraw->type == DRAWABLE_WINDOW)
    {
      if( /* check for being viewable */
	 !((WindowPtr) pDraw)->realized ||
	  /* check for being on screen */
         pDraw->x + stuff->x < 0 ||
 	 pDraw->x + stuff->x + (int)stuff->width > pDraw->pScreen->width ||
         pDraw->y + stuff->y < 0 ||
         pDraw->y + stuff->y + height > pDraw->pScreen->height ||
          /* check for being inside of border */
         stuff->x < - wBorderWidth((WindowPtr)pDraw) ||
         stuff->x + (int)stuff->width >
		wBorderWidth((WindowPtr)pDraw) + (int)pDraw->width ||
         stuff->y < -wBorderWidth((WindowPtr)pDraw) ||
         stuff->y + height >
		wBorderWidth ((WindowPtr)pDraw) + (int)pDraw->height
        )
	    return(BadMatch);
	*visual = wVisual (((WindowPtr) pDraw));
    }
    else
    {
      if(stuff->x < 0 ||
         stuff->x+(int)stuff->width > pDraw->width ||
         stuff->y < 0 ||
         stuff->y+height > pDraw->height
        )
	    return(BadMatch);
	*visual = None;
    }
    if(stuff->format == ZPixmap)
    {
	widthBytesLine = PixmapBytePad(stuff->width, pDraw->depth);
	length = widthBytesLine * height;
    }
    else 
    {
	widthBytesLine = BitmapBytePad(stuff->width);
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = widthBytesLine * height *
		 Ones(stuff->planeMask & (plane | (plane - 1)));
    }

    *imageBytes = length;

    if (widthBytesLine == 0 || height == 0)
	linesPerBuf = 0;
    else
	linesPerBuf = height;

    if(!(pBuf = *theImage = (char *) malloc(length + 4)))
        return (BadAlloc);

    if (linesPerBuf == 0)
    {
	/* nothing to do */
    }
    else if (stuff->format == ZPixmap)
    {
	(*pDraw->pScreen->GetImage) (pDraw,
	                                 stuff->x,
				         stuff->y,
				         stuff->width, 
				         height,
				         stuff->format,
				         stuff->planeMask,
				         (pointer) pBuf);
    }
    else /* XYPixmap */
    {
	char *bufptr = pBuf;

        for (; plane; plane >>= 1)
	{
	    if (stuff->planeMask & plane)
	    {
		(*pDraw->pScreen->GetImage) (pDraw,
	                                         stuff->x,
				                 stuff->y,
				                 stuff->width, 
				                 height,
				                 stuff->format,
				                 plane,
				                 (pointer)bufptr);

		    bufptr += (height * widthBytesLine);
            }
	}
    }

    return (client->noClientException);
}



int
LbxDecodeGetImage (client)

register ClientPtr  client;

{
    REQUEST		(xLbxGetImageReq);
    xLbxGetImageReply	*reply = NULL;
    int			depth, uncompLen, lbxLen, n;
    VisualID		visual;
    char		*theImage;
    int			method, bytes, status;

    REQUEST_SIZE_MATCH(xLbxGetImageReq);

    status = GetTheImage (client, stuff,
	&depth, &visual, &theImage, &uncompLen);

    if (status != Success)
	return (status);

    if ((reply = (xLbxGetImageReply *) xalloc (
	sz_xLbxGetImageReply + uncompLen)) == NULL)
    {
	return (!Success);
    }

    if (!((stuff->format == ZPixmap && depth == 8) || depth == 1))
    {
	status = LBX_IMAGE_COMPRESS_UNSUPPORTED_FORMAT;
    }
    else if (depth == 1)
    {
	status = LbxImageEncodeFaxG42D ((unsigned char *) theImage,
	      (unsigned char *) reply + sz_xLbxGetImageReply,
	      uncompLen, uncompLen, (int) stuff->width, &bytes);

	method = LbxImageCompressFaxG42D;
    }
    else /* depth 8 and ZPixmap format */
    {
	status = LbxImageEncodePackBits ((char *) theImage,
	      (char *) reply + sz_xLbxGetImageReply, uncompLen,
	      (int) stuff->height, (int) stuff->width, &bytes);

	method = LbxImageCompressPackBits;
    }

    reply->type = X_Reply;
    reply->depth = depth;
    reply->sequenceNumber = client->sequence;
    reply->visual = visual;

    if (status != LBX_IMAGE_COMPRESS_SUCCESS)
    {
	reply->compressionMethod = LbxImageCompressNone;
	reply->lbxLength = reply->xLength = (uncompLen + 3) >> 2;
    }
    else
    {
	reply->compressionMethod = method;
	reply->lbxLength = (bytes + 3) >> 2;
	reply->xLength = (uncompLen + 3) >> 2;
    }

    lbxLen = reply->lbxLength;

    if (client->swapped)
    {
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->lbxLength, n);
	swapl (&reply->xLength, n);
	swapl (&reply->visual, n);
    }

    if (reply->compressionMethod != LbxImageCompressNone)
    {
	UncompressedWriteToClient (client,
	    sz_xLbxGetImageReply + (lbxLen << 2), reply);
    }
    else
    {
	UncompressedWriteToClient (client, sz_xLbxGetImageReply, reply);
	UncompressedWriteToClient (client, lbxLen << 2, theImage);
    }

    free (theImage);

    if (reply)
	xfree ((char *) reply);

    return (Success);
}
