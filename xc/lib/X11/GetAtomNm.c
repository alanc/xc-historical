/* $XConsortium: XGetAtomNm.c,v 11.17 93/08/31 19:24:23 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
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

extern void _XFreeAtomTable();

/* XXX this table def is duplicated in XIntAtom.c, keep them consistent! */

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

char *XGetAtomName(dpy, atom)
register Display *dpy;
Atom atom;
{
    xGetAtomNameReply rep;
    xResourceReq *req;
    char *storage;
    register Entry *table;
    register int idx;
    register Entry e;
    register char *s1, c;
    register unsigned long sig;
    Entry oe;

    LockDisplay(dpy);
    /* look in the cache first */
    if (dpy->atoms) {
	table = dpy->atoms->table;
	for (idx = TABLESIZE; --idx >= 0; ) {
	    if ((e = *table++) && (e->atom == atom)) {
		idx = strlen(EntryName(e)) + 1;
		if (storage = (char *)Xmalloc(idx))
		    strcpy(storage, EntryName(e));
		UnlockDisplay(dpy);
		return storage;
	    }		
	}
    }
    GetResReq(GetAtomName, atom, req);
    if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) {
	UnlockDisplay(dpy);
	SyncHandle();
	return(NULL);
    }
    if (storage = (char *) Xmalloc(rep.nameLength+1)) {
	_XReadPad(dpy, storage, (long)rep.nameLength);
	storage[rep.nameLength] = '\0';
	/* store it in the cache */
	if (!dpy->atoms) {
	    dpy->atoms = (AtomTable *)Xcalloc(1, sizeof(AtomTable));
	    dpy->free_funcs->atoms = _XFreeAtomTable;
	}
	if (dpy->atoms) {
	    table = dpy->atoms->table;
	    sig = 0;
	    for (s1 = storage; c = *s1++; )
		sig = (sig << 1) + c;
	    idx = HASH(sig);
	    e = (Entry)Xmalloc(sizeof(EntryRec) + rep.nameLength + 1);
	    if (e) {
		e->sig = sig;
		e->atom = atom;
		strcpy(EntryName(e), storage);
		if (oe = table[idx])
		    Xfree((char *)oe);
		table[idx] = e;
	    }
	}
    } else {
	_XEatData(dpy, (unsigned long) (rep.nameLength + 3) & ~3);
	storage = (char *) NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return(storage);
}
