#include	"copyright.h"
/* @(#)FSCloseFt.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"


FSCloseFont(svr, fid)
    FSServer     *svr;
    Font        fid;
{
    fsResourceReq *req;

    GetResReq(CloseFont, fid, req);
    SyncHandle();
}
