#include	"copyright.h"
/* @(#)FSOpenFont.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

Font
FSOpenBitmapFont(svr, hint, fmask, name, originalid)
    FSServer   *svr;
    fsBitmapFormat hint;
    fsBitmapFormatMask fmask;
    char       *name;
    Font       *originalid;
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
    (void) _FSReply(svr, (fsReply *) & reply,
     (sizeof(fsOpenBitmapFontReply) - sizeof(fsGenericReply)) >> 2, fsFalse);
    *originalid = reply.originalid;
    SyncHandle();
    return fid;
}
