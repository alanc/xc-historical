#include	"copyright.h"
/* @(#)FSListExt.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

char      **
FSListExtensions(svr, next)
    FSServer     *svr;
    int        *next;
{
    fsListExtensionsReply rep;
    char      **list;
    char       *c;
    int         i,
                length;
    fsReq      *req;
    long        rlen;

    GetEmptyReq(ListExtensions, req);

    if (!_FSReply(svr, (fsReply *) & rep, 0, fsFalse)) {
	SyncHandle();
	return (char **) NULL;
    }
    if (rep.nExtensions) {
	list = (char **) FSmalloc((unsigned)(rep.nExtensions * sizeof(char *)));
	rlen = rep.length << 2;
	c = (char *) FSmalloc((unsigned) rlen + 1);
	if ((!list) || (!c)) {
	    if (list)
		FSfree((char *) list);
	    if (c)
		FSfree(c);
	    _FSEatData(svr, (unsigned long) rlen);
	    SyncHandle();
	    return (char **) NULL;
	}
	_FSReadPad(svr, c, rlen);
	/*
	 * unpack the strings
	 */
	length = *c;
	for (i = 0; i < rep.nExtensions; i++) {
	    list[i] = c + 1;	/* skip length */
	    c += length + 1;	/* find next length */
	    length = *c;
	    *c = '\0';		/* change length to NULL */
	}
    } else {
	list = (char **) NULL;
    }
    SyncHandle();
    *next = rep.nExtensions;
    return list;

}

FSFreeExtensionList(list)
    char      **list;
{
    if (list != NULL) {
	FSfree(list[0] - 1);
	FSfree((char *) list);
    }
}
