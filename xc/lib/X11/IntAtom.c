#include "copyright.h"

/* $Header: XIntAtom.c,v 11.9 87/06/10 23:06:15 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Atom XInternAtom (dpy, name, onlyIfExists)
    register Display *dpy;
    char *name;
    int onlyIfExists;
{
    register long nbytes;
    xInternAtomReply rep;
    register xInternAtomReq *req;
    LockDisplay(dpy);
    GetReq(InternAtom, req);
    nbytes = req->nbytes = strlen(name);
    req->onlyIfExists = onlyIfExists;
    req->length += (nbytes+3)>>2;
    _XSend (dpy, name, nbytes);
    	/* use _XSend instead of Data, since the following _XReply
           will always flush the buffer anyway */
    if(_XReply (dpy, (xReply *)&rep, 0, xTrue) == 0) rep.atom = None;
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.atom);
}

