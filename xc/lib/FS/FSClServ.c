#include	"copyright.h"
/* @(#)FSClServ.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

extern FSServer *_FSHeadOfServerList;

FSCloseServer(svr)
    FSServer     *svr;
{
    _FSExtension *ext;
    FSServer    **sv = &_FSHeadOfServerList;
    FSServer     *s = _FSHeadOfServerList;
    extern void _FSFreeQ();

    svr->flags |= FSlibServerClosing;
    FSSync(svr, 1);		/* throw out pending events */
    ext = svr->ext_procs;
    while (ext) {
	if (ext->close_server != NULL)
	    (*ext->close_server) (svr, &ext->codes);
	ext = ext->next;
    }
    _FSDisconnectServer(svr->fd);
    while (s != NULL) {
	if (s == svr) {
	    *sv = s->next;
	    _FSFreeServerStructure(svr);
	    break;
	}
	sv = &(s->next);
	s = *sv;
    }
    if (_FSHeadOfServerList == NULL) {
	_FSFreeQ();
    }
}
