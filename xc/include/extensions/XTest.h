/* $XConsortium: XTest.h,v 1.0 91/04/16 15:03:32 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

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

#ifndef _XTEST_H_
#define _XTEST_H_

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

Bool XTestQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* event_basep */,
    int*		/* error_basep */,
    int*		/* majorp */,
    int*		/* minorp */
#endif
);

Bool XTestCompareCursorWithWindow(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */,
    Cursor		/* cursor */
#endif
);

Bool XTestCompareCurrentCursorWithWindow(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */
#endif
);

void XTestSetGContextOfGC(
#if NeedFunctionPrototypes
    GC			/* gc */,
    GContext		/* gid */
#endif
);

void XTestSetVisualIDOfVisual(
#if NeedFunctionPrototypes
    Visual*		/* visual */,
    VisualID		/* visualid */
#endif
);

Status XTestDiscard(
#if NeedFunctionPrototypes
    Display*		/* dpy */
#endif
);

_XFUNCPROTOEND

#endif
