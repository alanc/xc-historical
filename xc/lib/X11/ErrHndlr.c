/* $XConsortium: ErrHndlr.c,v 11.18 93/08/14 17:43:41 rws Exp $ */
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

#include "Xlibint.h"

extern int _XDefaultError();
extern int _XDefaultIOError();
/* 
 * XErrorHandler - This procedure sets the X non-fatal error handler
 * (_XErrorFunction) to be the specified routine.  If NULL is passed in
 * the original error handler is restored.
 */
 
#if NeedFunctionPrototypes
XErrorHandler XSetErrorHandler(XErrorHandler handler)
#else
XErrorHandler XSetErrorHandler(handler)
    register XErrorHandler handler;
#endif
{
    int (*oldhandler)();

    LockMutex(_Xglobal_lock);
    oldhandler = _XErrorFunction;

    if (!oldhandler)
	oldhandler = _XDefaultError;

    if (handler != NULL) {
	_XErrorFunction = handler;
    }
    else {
	_XErrorFunction = _XDefaultError;
    }
    UnlockMutex(_Xglobal_lock);

    return (XErrorHandler) oldhandler;
}

/* 
 * XIOErrorHandler - This procedure sets the X fatal I/O error handler
 * (_XIOErrorFunction) to be the specified routine.  If NULL is passed in 
 * the original error handler is restored.
 */
 
extern int _XIOError();
#if NeedFunctionPrototypes
XIOErrorHandler XSetIOErrorHandler(XIOErrorHandler handler)
#else
XIOErrorHandler XSetIOErrorHandler(handler)
    register XIOErrorHandler handler;
#endif
{
    int (*oldhandler)();

    LockMutex(_Xglobal_lock);
    oldhandler = _XIOErrorFunction;

    if (!oldhandler)
	oldhandler = _XDefaultIOError;

    if (handler != NULL) {
	_XIOErrorFunction = handler;
    }
    else {
	_XIOErrorFunction = _XDefaultIOError;
    }
    UnlockMutex(_Xglobal_lock);

    return (XIOErrorHandler) oldhandler;
}
