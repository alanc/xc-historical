#include	"copyright.h"
/* @(#)FSQXExt.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

int
FSQueryXExtents8(svr, fid, range_type, str, str_len, extents)
    FSServer   *svr;
    Font        fid;
    Bool        range_type;
    unsigned char *str;
    unsigned long str_len;
    fsCharInfo **extents;
{
    fsQueryXExtents8Req *req;
    fsQueryXExtents8Reply reply;
    fsCharInfo *ext;
    int         i;

    GetReq(QueryXExtents8, req);
    req->fid = fid;
    req->range = range_type;
    req->num_ranges = str_len;
    req->length += (str_len + 3) >> 2;
    _FSSend(svr, (char *) str, str_len);

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
	       (sizeof(fsQueryXExtents8Reply) - sizeof(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

    ext = (fsCharInfo *) FSmalloc(sizeof(fsCharInfo) * reply.num_extents);
    *extents = ext;
    if (!ext)
	return FSBadAlloc;
    for (i = 0; i < reply.num_extents; i++) {
	_FSReadPad(svr, (char *) &ext[i], sizeof(fsCharInfo));
    }

    SyncHandle();
    return FSSuccess;
}

int
FSQueryXExtents16(svr, fid, range_type, str, str_len, extents)
    FSServer   *svr;
    Font        fid;
    Bool        range_type;
    fsChar2b   *str;
    unsigned long str_len;
    fsCharInfo **extents;
{
    fsQueryXExtents16Req *req;
    fsQueryXExtents16Reply reply;
    fsCharInfo *ext;
    int         i;

    GetReq(QueryXExtents16, req);
    req->fid = fid;
    req->range = range_type;
    req->num_ranges = str_len;
    req->length += ((str_len * sizeof(fsChar2b)) + 3) >> 2;
    _FSSend(svr, (char *) str, (str_len * sizeof(fsChar2b)));

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
	      (sizeof(fsQueryXExtents16Reply) - sizeof(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

    ext = (fsCharInfo *) FSmalloc(sizeof(fsCharInfo) * reply.num_extents);
    *extents = ext;
    if (!ext)
	return FSBadAlloc;
    for (i = 0; i < reply.num_extents; i++) {
	_FSReadPad(svr, (char *) &ext[i], sizeof(fsCharInfo));
    }

    SyncHandle();
    return FSSuccess;
}
