#include "copyright.h"

/* $Header: XModMap.c,v 11.1 87/05/30 14:35:56 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

XGetModifierMapping(dpy, modifier_keys)
     register Display *dpy;
     register XModifierKeys *modifier_keys;
{       
    xGetModifierMappingReply rep;
    register xReq *req;
    LockDisplay(dpy);
    GetEmptyReq(GetModifierMapping, req);
    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    modifier_keys->lock = rep.lock;
    modifier_keys->shift_a= rep.shiftA;
    modifier_keys->shift_b = rep.shiftB;
    modifier_keys->control_a = rep.controlA;
    modifier_keys->control_b = rep.controlB;
    modifier_keys->mod1_a = rep.mod1A;
    modifier_keys->mod1_b = rep.mod1B;
    modifier_keys->mod2_a = rep.mod2A;
    modifier_keys->mod2_b = rep.mod2B;
    modifier_keys->mod3_a = rep.mod3A;
    modifier_keys->mod3_b = rep.mod3B;
    modifier_keys->mod4_a = rep.mod4A;
    modifier_keys->mod4_b = rep.mod4B;
    modifier_keys->mod5_a = rep.mod5A;
    modifier_keys->mod5_b = rep.mod5B;
    UnlockDisplay(dpy);
    SyncHandle();
}


XSetModifierMapping(dpy, modifier_keys)
    register Display *dpy;
    register XModifierKeys *modifier_keys;
{       
    register xSetModifierMappingReq *req;

    LockDisplay(dpy);
    GetReq(SetModifierMapping, req);
    req->lock = modifier_keys->lock;
    req->shiftA = modifier_keys->shift_a;
    req->shiftB = modifier_keys->shift_b;
    req->controlA = modifier_keys->control_a;
    req->controlB = modifier_keys->control_b;
    req->mod1A = modifier_keys->mod1_a;
    req->mod1B = modifier_keys->mod1_b;
    req->mod2A = modifier_keys->mod2_a;
    req->mod2B = modifier_keys->mod2_b;
    req->mod3A = modifier_keys->mod3_a;
    req->mod3B = modifier_keys->mod3_b;
    req->mod4A = modifier_keys->mod4_a;
    req->mod4B = modifier_keys->mod4_b;
    req->mod5A = modifier_keys->mod5_a;
    req->mod5B = modifier_keys->mod5_b;
    UnlockDisplay(dpy);
    SyncHandle();
}

