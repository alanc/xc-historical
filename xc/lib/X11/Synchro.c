/* $XConsortium: Synchro.c,v 11.10 91/11/09 15:40:03 keith Exp $ */
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

int _XSyncFunction(dpy)
register Display *dpy;
{
    xGetInputFocusReply rep;
    register xReq *req;
    int (*func)();

    LockDisplay(dpy);
    GetEmptyReq(GetInputFocus, req);
    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    if (func = dpy->savedsynchandler) {
	dpy->synchandler = func;
	dpy->savedsynchandler = NULL;
    }
    UnlockDisplay(dpy);
    if (func) (*func)(dpy);
    return 0;
}

#if NeedFunctionPrototypes
int (*XSynchronize(Display *dpy, int onoff))()
#else
int (*XSynchronize(dpy,onoff))()
     register Display *dpy;
     int onoff;
#endif
{
        int (*temp)();
	int (*func)() = NULL;

	if (onoff)
	    func = _XSyncFunction;

	LockDisplay(dpy);
	if (temp = dpy->savedsynchandler) {
	    dpy->savedsynchandler = func;
	} else {
	    temp = dpy->synchandler;
	    dpy->synchandler = func;
	}
	UnlockDisplay(dpy);
	return (temp);
}

#if NeedFunctionPrototypes
int (*XSetAfterFunction(
     Display *dpy,
     int (*func)(
#if NeedNestedPrototypes
		 Display*
#endif
		 )
))()
#else
int (*XSetAfterFunction(dpy,func))()
     register Display *dpy;
     int (*func)(
#if NeedNestedPrototypes
		 Display*
#endif
		 );
#endif
{
        int (*temp)();

	LockDisplay(dpy);
	if (temp = dpy->savedsynchandler) {
	    dpy->savedsynchandler = func;
	} else {
	    temp = dpy->synchandler;
	    dpy->synchandler = func;
	}
	UnlockDisplay(dpy);
	return (temp);
}

