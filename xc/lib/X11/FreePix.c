#include "copyright.h"

/* $Header: XFreePix.c,v 11.5 87/05/24 21:35:28 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XFreePixmap(dpy, pixmap)
    register Display *dpy;
    Pixmap pixmap;

{   
    register xResourceReq *req;    
    LockDisplay(dpy);
    GetResReq(FreePixmap, pixmap, req);
    UnlockDisplay(dpy);
    SyncHandle();
}

