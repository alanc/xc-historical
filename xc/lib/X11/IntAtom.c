/* $XConsortium: IntAtom.c,v 11.23 93/09/22 09:45:55 rws Exp $ */
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

/* XXX this table def is duplicated in GetAtomNm.c, keep them consistent! */

#define TABLESIZE 64

typedef struct _Entry {
    unsigned long sig;
    Atom atom;
} EntryRec, *Entry;

#define EntryName(e) ((char *)(e+1))

typedef struct _XDisplayAtoms {
    Entry table[TABLESIZE];
} AtomTable;

#define HASH(sig) ((sig) & (TABLESIZE-1))
#define REHASHVAL(sig) ((((sig) % (TABLESIZE-3)) + 2) | 1)
#define REHASH(idx,rehash) ((idx + rehash) & (TABLESIZE-1))

void
_XFreeAtomTable(dpy)
    Display *dpy;
{
    register Entry *table;
    register int i;
    register Entry e;

    if (dpy->atoms) {
	table = dpy->atoms->table;
	for (i = TABLESIZE; --i >= 0; ) {
	    if (e = *table++)
		Xfree((char *)e);
	}
	Xfree((char *)dpy->atoms);
    }
}

static
Atom _XInternAtom(dpy, name, onlyIfExists, psig, pidx, pn)
    Display *dpy;
    char *name;
    Bool onlyIfExists;
    unsigned long *psig;
    int *pidx;
    int *pn;
{
    register AtomTable *atoms;
    register char *s1, c, *s2;
    register unsigned long sig;
    register int idx, i;
    Entry e;
    int n, firstidx, rehash;
    xInternAtomReq *req;

    if (!name)
	name = "";
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
	while (e = atoms->table[idx]) {
	    if (e->sig == sig) {
	    	for (i = n, s1 = (char *)name, s2 = EntryName(e); --i >= 0; ) {
		    if (*s1++ != *s2++)
		    	goto nomatch;
	    	}
	    	if (!*s2)
		    return e->atom;
	    }
nomatch:    if (idx == firstidx)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    if (idx == firstidx)
		break;
	}
    }
    *psig = sig;
    *pidx = idx;
    *pn = n;
    /* not found, go to the server */
    GetReq(InternAtom, req);
    req->nbytes = n;
    req->onlyIfExists = onlyIfExists;
    req->length += (n+3)>>2;
    _XSend (dpy, name, n);
    	/* use _XSend instead of Data, since the following _XReply
           will always flush the buffer anyway */
    return None;
}

void
_XUpdateAtomCache(dpy, name, atom, sig, idx, n)
    Display *dpy;
    char *name;
    Atom atom;
    unsigned long sig;
    int idx;
    int n;
{
    Entry e, oe;
    register char *s1;
    register char c;
    int firstidx, rehash;

    if (!dpy->atoms) {
	if (idx < 0) {
	    dpy->atoms = (AtomTable *)Xcalloc(1, sizeof(AtomTable));
	    dpy->free_funcs->atoms = _XFreeAtomTable;
	}
	if (!dpy->atoms)
	    return;
    }
    if (!sig) {
	for (s1 = (char *)name; c = *s1++; )
	    sig = (sig << 1) + c;
	n = s1 - (char *)name - 1;
	if (idx < 0) {
	    firstidx = idx = HASH(sig);
	    if (dpy->atoms->table[idx]) {
		rehash = REHASHVAL(sig);
		do
		    idx = REHASH(idx, rehash);
		while (idx != firstidx && dpy->atoms->table[idx]);
	    }
	}
    }
    e = (Entry)Xmalloc(sizeof(EntryRec) + n + 1);
    if (e) {
	e->sig = sig;
	e->atom = atom;
	strcpy(EntryName(e), name);
	if (oe = dpy->atoms->table[idx])
	    Xfree((char *)oe);
	dpy->atoms->table[idx] = e;
    }
}

#if NeedFunctionPrototypes
Atom XInternAtom (
    Display *dpy,
    _Xconst char *name,
    Bool onlyIfExists)
#else
Atom XInternAtom (dpy, name, onlyIfExists)
    Display *dpy;
    char *name;
    Bool onlyIfExists;
#endif
{
    Atom atom;
    unsigned long sig;
    int idx, n;
    xInternAtomReply rep;

    LockDisplay(dpy);
    if (atom = _XInternAtom(dpy, name, onlyIfExists, &sig, &idx, &n)) {
	UnlockDisplay(dpy);
	return atom;
    }
    if (_XReply (dpy, (xReply *)&rep, 0, xTrue)) {
	if (atom = rep.atom)
	    _XUpdateAtomCache(dpy, name, atom, sig, idx, n);
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.atom);
}

typedef struct {
    unsigned long start_seq;
    unsigned long stop_seq;
    char **names;
    Atom *atoms;
    int count;
    Status status;
} _XIntAtomState;

static
Bool _XIntAtomHandler(dpy, rep, buf, len, data)
    register Display *dpy;
    register xReply *rep;
    char *buf;
    int len;
    XPointer data;
{
    register _XIntAtomState *state;
    register int i, idx;
    xInternAtomReply replbuf;
    register xInternAtomReply *repl;

    state = (_XIntAtomState *)data;
    if (dpy->last_request_read < state->start_seq ||
	dpy->last_request_read > state->stop_seq)
	return False;
    for (i = 0; i < state->count; i++) {
	if (state->atoms[i] & 0x80000000) {
	    idx = ~state->atoms[i];
	    state->atoms[i] = None;
	    break;
	}
    }
    if (i >= state->count)
	return False;
    if (rep->generic.type == X_Error) {
	state->status = 0;
	return False;
    }
    repl = (xInternAtomReply *)
	_XGetAsyncReply(dpy, (char *)&replbuf, rep, buf, len,
			(SIZEOF(xInternAtomReply) - SIZEOF(xReply)) >> 2,
			True);
    if (state->atoms[i] = repl->atom)
	_XUpdateAtomCache(dpy, state->names[i], repl->atom,
			  (unsigned long)0, idx, 0);
    return True;
}

Status
XInternAtoms (dpy, names, count, onlyIfExists, atoms_return)
    Display *dpy;
    char **names;
    int count;
    Bool onlyIfExists;
    Atom *atoms_return;
{
    int i, idx, n;
    unsigned long sig;
    _XAsyncHandler async;
    _XIntAtomState async_state;
    int missed = -1;
    xInternAtomReply rep;

    LockDisplay(dpy);
    async_state.start_seq = dpy->request + 1;
    async_state.atoms = atoms_return;
    async_state.names = names;
    async_state.count = count - 1;
    async_state.status = 1;
    async.next = dpy->async_handlers;
    async.handler = _XIntAtomHandler;
    async.data = (XPointer)&async_state;
    dpy->async_handlers = &async;
    for (i = 0; i < count; i++) {
	if (!(atoms_return[i] = _XInternAtom(dpy, names[i], onlyIfExists,
					     &sig, &idx, &n))) {
	    missed = i;
	    atoms_return[i] = ~((Atom)idx);
	    async_state.stop_seq = dpy->request;
	}
    }
    if (missed >= 0) {
	if (_XReply (dpy, (xReply *)&rep, 0, xTrue)) {
	    if (atoms_return[missed] = rep.atom)
		_XUpdateAtomCache(dpy, names[missed], rep.atom, sig, idx, n);
	} else {
	    atoms_return[missed] = None;
	    async_state.status = 0;
	}
    }
    DeqAsyncHandler(dpy, &async);
    UnlockDisplay(dpy);
    if (missed >= 0)
	SyncHandle();
    return async_state.status;
}
