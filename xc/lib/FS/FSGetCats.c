/* $XConsortium: FSGetCats.c,v 1.1 91/07/16 20:31:55 keith Exp $ */
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

#include	"FSlibint.h"

char      **
FSGetCatalogues(svr, num)
    FSServer   *svr;
    int        *num;
{
    fsGetCataloguesReply rep;
    char      **list;
    char       *c;
    int         i,
                length;
    fsReq      *req;
    long        rlen;

    GetEmptyReq(GetCatalogues, req);

    if (!_FSReply(svr, (fsReply *) & rep, 0, fsFalse)) {
	SyncHandle();
	return (char **) NULL;
    }
    if (rep.num_catalogues) {
	list = (char **)
	    FSmalloc((unsigned) (rep.num_catalogues * sizeof(char *)));
	rlen = (rep.length << 2) - SIZEOF(fsGetCataloguesReply);
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
	for (i = 0; i < (int)rep.num_catalogues; i++) {
	    list[i] = c + 1;	/* skip length */
	    c += length + 1;	/* find next length */
	    length = *c;
	    *c = '\0';		/* change length to NULL */
	}
    } else {
	list = (char **) NULL;
    }
    SyncHandle();
    *num = rep.num_catalogues;
    return list;
}
