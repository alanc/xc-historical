#include "copyright.h"

/* $Header: XGetKCnt.c,v 11.9 88/08/09 15:56:58 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

struct kmap {char keys[32];};

void
XGetKeyboardControl (dpy, state)
    register Display *dpy;
    register XKeyboardState *state;
    {
    xGetKeyboardControlReply rep;
    register xReq *req;
    LockDisplay(dpy);
    GetEmptyReq (GetKeyboardControl, req);
    (void) _XReply (dpy, (xReply *) &rep, 
	(sizeof(xGetKeyboardControlReply) - SIZEOF(xReply)) >> 2, xTrue);

    state->key_click_percent = rep.keyClickPercent;
    state->bell_percent = rep.bellPercent;
    state->bell_pitch = rep.bellPitch;
    state->bell_duration = rep.bellDuration;
    state->led_mask = rep.ledMask;
    state->global_auto_repeat = rep.globalAutoRepeat;
    * (struct kmap *) state->auto_repeats = * (struct kmap *) rep.map;
    UnlockDisplay(dpy);
    SyncHandle();
    }

