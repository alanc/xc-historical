#include "copyright.h"

/* $Header: XGrKey.c,v 11.5 87/05/24 21:36:12 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XGrabKey(dpy, key, modifiers, grab_window, owner_events, 
	 pointer_mode, keyboard_mode)
    register Display *dpy;
    int key;
    unsigned int modifiers;
    Window grab_window;
    int owner_events;  /* boolean */
    int pointer_mode, keyboard_mode;

{
    register xGrabKeyReq *req;
    LockDisplay(dpy);
    GetReq(GrabKey, req);
    req->ownerEvents = owner_events;
    req->grabWindow = grab_window;
    req->modifiers = modifiers;
    req->key = key;
    req->pointerMode = pointer_mode;
    req->keyboardMode = keyboard_mode;
    UnlockDisplay(dpy);
    SyncHandle();
}



