#include	"copyright.h"
/* @(#)FSSync.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

extern _FSQEvent *_qfree;

/* synchronize withe errors and events */

FSSync(svr, discard)
    FSServer     *svr;
    Bool        discard;
{
    fsListExtensionsReply rep;
    fsReq      *req;

    GetEmptyReq(ListExtensions, req);
    (void) _FSReply(svr, (fsReply *) & rep, 0, fsTrue);
    if (rep.nExtensions) {
	_FSEatData(svr, (unsigned long) rep.length << 2);
    }

    if (discard && svr->head) {
	((_FSQEvent *) svr->tail)->next = _qfree;
	_qfree = (_FSQEvent *) svr->head;
	svr->head = svr->tail = NULL;
	svr->qlen = 0;
    }
}
