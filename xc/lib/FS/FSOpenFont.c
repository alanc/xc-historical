/* $XConsortium: FSOpenFont.c,v 1.4 92/11/18 21:31:17 gildea Exp $ */
/*
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include "FSlibint.h"

Font
FSOpenBitmapFont(svr, hint, fmask, name, otherid)
    FSServer   *svr;
    FSBitmapFormat hint;
    FSBitmapFormatMask fmask;
    char       *name;
    Font       *otherid;
{
    unsigned char nbytes;
    fsOpenBitmapFontReq *req;
    fsOpenBitmapFontReply reply;
    Font        fid;
    char        buf[256];

    GetReq(OpenBitmapFont, req);
    nbytes = name ? strlen(name) : 0;
    buf[0] = (char) nbytes;
    bcopy(name, &buf[1], nbytes);
    nbytes++;
    req->fid = fid = svr->resource_id++;
    req->format_hint = hint;
    req->format_mask = fmask;
    req->length += (nbytes + 3) >> 2;
    _FSSend(svr, buf, (long) nbytes);
    if (!_FSReply(svr, (fsReply *) & reply,
		  (SIZEOF(fsOpenBitmapFontReply)-SIZEOF(fsGenericReply)) >> 2,
		  fsFalse))
	return 0;
    *otherid = reply.otherid;
    SyncHandle();
    return fid;
}
