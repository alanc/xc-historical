#include "copyright.h"

/* $Header: XUnldFont.c,v 11.5 87/05/24 21:40:38 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XUnloadFont(dpy, font)
     register Display *dpy;
     Font font;

{       
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(CloseFont, font, req);
    UnlockDisplay(dpy);
    SyncHandle();
}

