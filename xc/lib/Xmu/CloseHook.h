/*
 * $XConsortium: CloseHook.h,v 1.3 91/05/28 14:55:34 converse Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_CLOSEHOOK_H_
#define _XMU_CLOSEHOOK_H_

typedef caddr_t CloseHook;

typedef int (*XmuCloseHookProc)(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    caddr_t	/* data */
#endif 
);

#ifdef __cplusplus
extern "C" {					/* for C++ V2.0 */
#endif

extern CloseHook XmuAddCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg */
#endif
);

extern Bool XmuLookupCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    CloseHook	/* handle */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg  */
#endif
);

extern Bool XmuRemoveCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    CloseHook	/* handle */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg */
#endif
);

#ifdef __cplusplus
}						/* for C++ V2.0 */
#endif

#endif /* _XMU_CLOSEHOOK_H_ */
