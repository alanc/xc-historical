/* $XConsortium: GetAtomNm.c,v 11.20 93/09/22 09:45:17 rws Exp $ */
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

/* XXX this table def is duplicated in IntAtom.c, keep them consistent! */

#define TABLESIZE 64

typedef struct _Entry {
    unsigned long sig;
    Atom atom;
} EntryRec, *Entry;

#define EntryName(e) ((char *)(e+1))

typedef struct _XDisplayAtoms {
    Entry table[TABLESIZE];
} AtomTable;

static
char *_XGetAtomName(dpy, atom)
    Display *dpy;
    Atom atom;
{
    xResourceReq *req;
    char *name;
    register Entry *table;
    register int idx;
    register Entry e;

    if (dpy->atoms) {
	table = dpy->atoms->table;
	for (idx = TABLESIZE; --idx >= 0; ) {
	    if ((e = *table++) && (e->atom == atom)) {
		idx = strlen(EntryName(e)) + 1;
		if (name = (char *)Xmalloc(idx))
		    strcpy(name, EntryName(e));
		return name;
	    }		
	}
    }
    GetResReq(GetAtomName, atom, req);
    return (char *)NULL;
}

char *XGetAtomName(dpy, atom)
    register Display *dpy;
    Atom atom;
{
    xGetAtomNameReply rep;
    char *name;

    LockDisplay(dpy);
    if (name = _XGetAtomName(dpy, atom)) {
	UnlockDisplay(dpy);
	return name;
    }	
    if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) {
	UnlockDisplay(dpy);
	SyncHandle();
	return(NULL);
    }
    if (name = (char *) Xmalloc(rep.nameLength+1)) {
	_XReadPad(dpy, name, (long)rep.nameLength);
	name[rep.nameLength] = '\0';
	_XUpdateAtomCache(dpy, name, atom, 0, -1, 0);
    } else {
	_XEatData(dpy, (unsigned long) (rep.nameLength + 3) & ~3);
	name = (char *) NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return(name);
}

typedef struct {
    unsigned long start_seq;
    unsigned long stop_seq;
    Atom *atoms;
    char **names;
    int idx;
    int count;
    Status status;
} _XGetAtomNameState;

static
Bool _XGetAtomNameHandler(dpy, rep, buf, len, data)
    register Display *dpy;
    register xReply *rep;
    char *buf;
    int len;
    XPointer data;
{
    register _XGetAtomNameState *state;
    xGetAtomNameReply replbuf;
    register xGetAtomNameReply *repl;

    state = (_XGetAtomNameState *)data;
    if (dpy->last_request_read < state->start_seq ||
	dpy->last_request_read > state->stop_seq)
	return False;
    while (state->idx < state->count && state->names[state->idx])
	state->idx++;
    if (state->idx >= state->count)
	return False;
    if (rep->generic.type == X_Error) {
	state->status = 0;
	return False;
    }
    repl = (xGetAtomNameReply *)
	_XGetAsyncReply(dpy, (char *)&replbuf, rep, buf, len,
			(SIZEOF(xGetAtomNameReply) - SIZEOF(xReply)) >> 2,
			False);
    state->names[state->idx] = (char *) Xmalloc(repl->nameLength+1);
    _XGetAsyncData(dpy, state->names[state->idx], buf, len,
		   SIZEOF(xGetAtomNameReply), repl->nameLength,
		   repl->length << 2);
    _XUpdateAtomCache(dpy, state->names[state->idx],
		      state->atoms[state->idx], 0, -1, 0);
    return True;
}

Status
XGetAtomNames (dpy, atoms, count, names_return)
    Display *dpy;
    Atom *atoms;
    int count;
    char **names_return;
{
    _XAsyncHandler async;
    _XGetAtomNameState async_state;
    xGetAtomNameReply rep;
    int i;
    int missed = -1;

    LockDisplay(dpy);
    async_state.start_seq = dpy->request + 1;
    async_state.atoms = atoms;
    async_state.names = names_return;
    async_state.idx = 0;
    async_state.count = count - 1;
    async_state.status = 1;
    async.next = dpy->async_handlers;
    async.handler = _XGetAtomNameHandler;
    async.data = (XPointer)&async_state;
    dpy->async_handlers = &async;
    for (i = 0; i < count; i++) {
	if (!(names_return[i] = _XGetAtomName(dpy, atoms[i]))) {
	    missed = i;
	    async_state.stop_seq = dpy->request;
	}
    }
    if (missed >= 0) {
	if (_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	    if (names_return[missed] = (char *) Xmalloc(rep.nameLength+1)) {
		_XReadPad(dpy, names_return[missed], (long)rep.nameLength);
		names_return[missed][rep.nameLength] = '\0';
		_XUpdateAtomCache(dpy, names_return[missed], atoms[missed],
				  0, -1, 0);
	    } else {
		_XEatData(dpy, (unsigned long) (rep.nameLength + 3) & ~3);
		async_state.status = 0;
	    }
	}
    }
    DeqAsyncHandler(dpy, &async);
    UnlockDisplay(dpy);
    if (missed >= 0)
	SyncHandle();
    return async_state.status;
}
