/*
 * $XConsortium: scrnsaver.h,v 1.1 92/02/13 16:09:00 keith Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _SCRNSAVER_H_
#define _SCRNSAVER_H_

#include <X11/Xfuncproto.h>
#include <X11/extensions/saver.h>

typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    Bool send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Window window;	    /* screen saver window */
    Window root;	    /* root window of event screen */
    int state;		    /* ScreenSaverOff, ScreenSaverOn, ScreenSaverCycle*/
    int kind;		    /* ScreenSaverBlanked, ...Internal, ...External */
    Bool forced;	    /* extents of new region */
    Time time;		    /* event timestamp */
} XScreenSaverNotifyEvent;

typedef struct {
    Window  window;
    int	    state;
    int	    kind;
    Time    til_or_since;
    Time    idle;
    unsigned long   eventMask;
} XScreenSaverInfo;

_XFUNCPROTOBEGIN

extern Bool XScreenSaverQueryExtension (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int*	/* event_base */,
    int*	/* error_base */
#endif
);

extern Status XScreenSaverQueryVersion (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int*	/* major_version */,
    int*	/* minor_version */
#endif
);

extern XScreenSaverInfo *XScreenSaverAllocateInfo (
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);

extern Status XScreenSaverQueryInfo (
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* drawable */,
    XScreenSaverInfo*	/* info */
#endif
);

extern void XScreenSaverSelectInput (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Drawable	/* drawable */,
    unsigned long   /* eventMask */
#endif
);

extern void XScreenSaverSetAttributes (
#if NeedFunctionPrototypes
    Display*		    /* display */,
    Drawable		    /* drawable */,
    int			    /* x */,
    int			    /* y */,
    unsigned int	    /* width */,
    unsigned int	    /* height */,
    unsigned int	    /* border_width */,
    int			    /* depth */,
    unsigned int	    /* class */,
    Visual *		    /* visual */,
    unsigned long	    /* valuemask */,
    XSetWindowAttributes *  /* attributes */
#endif
);

extern void XScreenSaverUnsetAttributes (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Drawable	/* drawable */
#endif
);

extern Status XScreenSaverRegister (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int		/* screen */,
    XID		/* xid */,
    Atom	/* type */
#endif
);

extern Status XScreenSaverUnregister (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int		/* screen */
#endif
);

extern Status XScreenSaverGetRegistered (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int		/* screen */,
    XID*	/* xid */,
    Atom*	/* type */
#endif
);

_XFUNCPROTOEND

#endif /* _SCRNSAVER_H_ */
