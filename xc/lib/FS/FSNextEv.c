#include	"copyright.h"
/* @(#)FSNextEv.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

extern _FSQEvent *_qfree;

FSNextEvent(svr, event)
    FSServer     *svr;
    FSEvent    *event;
{
    _FSQEvent  *qelt;

    if (svr->head == NULL)
	_FSReadEvents(svr);
    qelt = svr->head;
    *event = qelt->event;

    if ((svr->head = qelt->next) == NULL)
	svr->tail = NULL;
    qelt->next = _qfree;
    _qfree = qelt;
    svr->qlen--;
}
