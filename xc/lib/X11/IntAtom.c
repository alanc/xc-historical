/* $XConsortium: XIntAtom.c,v 11.13 89/12/11 19:09:36 rws Exp $ */
/*

Copyright 1986, 1990 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#define NEED_REPLIES
#include "Xlibint.h"

#define TABLESIZE 64

typedef struct _XDisplayAtoms {
    char *name[TABLESIZE];
    unsigned long sig[TABLESIZE];
    Atom atom[TABLESIZE];
} AtomTable;

#define HASH(sig) ((sig) & (TABLESIZE-1))
#define REHASHVAL(sig) ((((sig) % (TABLESIZE-3)) + 2) | 1)
#define REHASH(idx,rehash) idx = ((idx + rehash) & (TABLESIZE-1))

static void
_XFreeAtomTable(dpy)
    Display *dpy;
{
    register AtomTable *atoms;
    register int i;
    register char *s;

    if (atoms = dpy->atoms) {
	for (i = TABLESIZE; --i >= 0; ) {
	    if (s = atoms->name[i])
		Xfree(s);
	}
	Xfree((char *)atoms);
    }
}

#if NeedFunctionPrototypes
Atom XInternAtom (
    Display *dpy,
    const char *name,
    Bool onlyIfExists)
#else
Atom XInternAtom (dpy, name, onlyIfExists)
    Display *dpy;
    char *name;
    Bool onlyIfExists;
#endif
{
    register AtomTable *atoms;
    register char *s1, c, *s2;
    register unsigned long sig;
    register int idx, i;
    int n, firstidx, rehash;
    xInternAtomReply rep;
    xInternAtomReq *req;

    LockDisplay(dpy);
    /* look in the cache first */
    if (!(atoms = dpy->atoms)) {
	dpy->atoms = atoms = (AtomTable *)Xcalloc(1, sizeof(AtomTable));
	dpy->free_funcs->atoms = _XFreeAtomTable;
    }
    sig = 0;
    for (s1 = (char *)name; c = *s1++; )
	sig = (sig << 1) + c;
    n = s1 - (char *)name - 1;
    if (atoms) {
	firstidx = idx = HASH(sig);
	while (s2 = atoms->name[idx]) {
	    if (atoms->sig[idx] == sig) {
	    	for (i = n, s1 = (char *)name; --i >= 0; ) {
		    if (*s1++ != *s2++)
		    	goto nomatch;
	    	}
	    	if (!*s2) {
		    rep.atom = atoms->atom[idx];
		    UnlockDisplay(dpy);
		    return rep.atom;
	    	}
	    }
nomatch:    if (idx == firstidx)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    if (idx == firstidx)
		break;
	}
    }
    /* not found, go to the server */
    GetReq(InternAtom, req);
    req->nbytes = n;
    req->onlyIfExists = onlyIfExists;
    req->length += (n+3)>>2;
    _XSend (dpy, name, n);
    	/* use _XSend instead of Data, since the following _XReply
           will always flush the buffer anyway */
    if(_XReply (dpy, (xReply *)&rep, 0, xTrue) == 0) {
	rep.atom = None;
    } else if (atoms) {
	/* store it in the cache */
	s1 = Xmalloc(n + 1);
	if (s1) {
	    strcpy(s1, name);
	    if (s2 = atoms->name[idx])
		Xfree(s2);
	    atoms->name[idx] = s1;
	    atoms->atom[idx] = rep.atom;
	    atoms->sig[idx] = sig;
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.atom);
}
