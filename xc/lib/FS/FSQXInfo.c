#include	"copyright.h"
/* @(#)FSQXInfo.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

/*
 * Note:  only the range in the first FSQuery is sent to the server.
 * the others exist as return values only.
 */

int
FSQueryXInfo(svr, fid, info, props, offsets, prop_data)
    FSServer   *svr;
    Font        fid;
    fsFontHeader *info;
    fsPropInfo *props;
    fsPropOffset **offsets;
    unsigned char **prop_data;
{
    fsQueryXInfoReq *req;
    fsQueryXInfoReply reply;
    fsPropOffset *offset_data;
    unsigned char *pdata;

    GetReq(QueryXInfo, req);
    req->id = fid;

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply, ((SIZEOF(fsQueryXInfoReply) -
			    SIZEOF(fsGenericReply)) >> 2), fsFalse)) {
	return FSBadAlloc;
    }
    bcopy((char *) &reply.header, (char *) info, sizeof(fsFontHeader));
    /* get the prop header */
    _FSReadPad(svr, (char *) props, sizeof(fsPropInfo));
    /* prepare for prop data */
    offset_data = (fsPropOffset *)
	FSmalloc(props->num_offsets * sizeof(fsPropOffset));
    if (!offset_data)
	return FSBadAlloc;
    pdata = (unsigned char *) FSmalloc(props->data_len);
    if (!pdata) {
	FSfree((char *) offset_data);
	return FSBadAlloc;
    }
    /* get offsets */
    _FSReadPad(svr, (char *) offset_data,
	       (props->num_offsets * sizeof(fsPropOffset)));
    /* get data */
    _FSReadPad(svr, (char *) pdata, props->data_len);
    *offsets = offset_data;
    *prop_data = pdata;

    SyncHandle();
    return FSSuccess;
}
