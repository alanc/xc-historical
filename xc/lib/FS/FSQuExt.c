#include	"copyright.h"
/* @(#)FSQuExt.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

Bool
FSQueryExtension(svr, name, major_opcode, first_event, first_error)
    FSServer   *svr;
    char       *name;
    int        *major_opcode;
    int        *first_event;
    int        *first_error;
{
    fsQueryExtensionReply rep;
    fsQueryExtensionReq *req;

    GetReq(QueryExtension, req);
    req->nbytes = name ? strlen(name) : 0;
    req->length += (req->nbytes + 3) >> 2;
    _FSSend(svr, name, (long) req->nbytes);
    if (!_FSReply(svr, (fsReply *) & rep,
      (sizeof(fsQueryExtensionReply) - sizeof(fsGenericReply)) >> 2, fsFalse))
	return FSBadAlloc;
    *major_opcode = rep.major_opcode;
    *first_event = rep.first_event;
    *first_error = rep.first_error;
    SyncHandle();
    return (rep.present);
}
