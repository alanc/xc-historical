#include "copyright.h"

/* $XConsortium: XQuKeybd.c,v 11.9 88/08/09 15:57:24 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

struct kmap {
  char map[32];
};

XQueryKeymap(dpy, keys)
    register Display *dpy;
    char keys[32];

{       
    xQueryKeymapReply rep;
    register xReq *req;

    LockDisplay(dpy);
    GetEmptyReq(QueryKeymap, req);
    (void) _XReply(dpy, (xReply *)&rep, 
       (SIZEOF(xQueryKeymapReply) - SIZEOF(xReply)) >> 2, xTrue);
    *(struct kmap *) keys = *(struct kmap *)rep.map;  /* faster than bcopy */
    UnlockDisplay(dpy);
    SyncHandle();
}

