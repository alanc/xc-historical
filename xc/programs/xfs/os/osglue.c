/* $XConsortium$ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)osglue.c	4.1	91/05/02
 *
 */
#include	"osstruct.h"

static int  num_alts;
static AlternateServerPtr alt_servers = (AlternateServerPtr) 0;

int
ListCatalogues(catalogues, len)
    char      **catalogues;
    int        *len;
{
    *len = 0;
    *catalogues = (char *) 0;
    return 0;
}

/* ARGSUSED */
int
ValidateCatalogues(cats)
    char       *cats;
{
    /* XXX -- since catalogue names aren't supported */
    return FSBadName;
}

int
SetAlternateServers(list)
    char       *list;
{
    char       *t,
               *st;
    AlternateServerPtr alts,
                a;
    int         num,
                i;

    t = list;
    num = 1;
    while (*t) {
	if (*t == ',')
	    num++;
	t++;
    }

    a = alts = (AlternateServerPtr) fsalloc(sizeof(AlternateServerRec) * num);
    if (!alts)
	return FSBadAlloc;

    st = t = list;
    a->namelen = 0;
    while (*t) {
	if (*t == ',') {
	    a->name = (char *) fsalloc(a->namelen);
	    if (!a->name) {
		/* XXX  -- leak */
		return FSBadAlloc;
	    }
	    bcopy(st, a->name, a->namelen);
	    a->subset = FALSE;	/* XXX */
	    a++;
	    t++;
	    st = t;
	    a->namelen = 0;
	} else {
	    a->namelen++;
	    t++;
	}
    }
    a->name = (char *) fsalloc(a->namelen);
    if (!a->name) {
	/* XXX  -- leak */
	return FSBadAlloc;
    }
    bcopy(st, a->name, a->namelen);
    a->subset = FALSE;	/* XXX */

    for (i = 0; i < num_alts; i++) {
	fsfree((char *) alt_servers[i].name);
    }
    fsfree((char *) alt_servers);
    num_alts = num;
    alt_servers = alts;
    return FSSuccess;
}

int
ListAlternateServers(svrs)
    AlternateServerPtr *svrs;
{
    *svrs = alt_servers;
    return num_alts;
}
