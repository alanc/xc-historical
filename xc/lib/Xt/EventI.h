/* $XConsortium: EventI.h,v 1.12 90/04/13 20:12:02 swick Exp $ */
/* $oHeader: EventI.h,v 1.3 88/08/24 09:21:11 asente Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* 
 * Event.h - exported types and functions for toolkit event handler
 * 
 * Author:	Charles Haynes
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Sun Dec  6 1987
 */

#ifndef _Event_h_
#define _Event_h_

typedef struct _XtGrabRec  *XtGrabList;

extern void _XtEventInitialize(
#if NeedFunctionPrototypes
    void
#endif
);

extern void _XtRegisterWindow(
#if NeedFunctionPrototypes
    Window 	/* window */,
    Widget 	/* widget */
#endif
);

extern void _XtUnregisterWindow(
#if NeedFunctionPrototypes
    Window 	/* window */,
    Widget 	/* widget */
#endif
);

typedef struct _XtEventRec {
     XtEventTable	next;
     EventMask		mask;
     Boolean		non_filter;
     Boolean		select;
     Boolean		raw;
     Boolean		async;
     XtEventHandler	proc;
     XtPointer		closure;
} XtEventRec;

typedef struct _XtGrabRec {
    XtGrabList next;
    Widget   widget;
    Boolean  exclusive;
    Boolean  spring_loaded;
}XtGrabRec;


extern void _XtFreeEventTable(
#if NeedFunctionPrototypes
    XtEventTable*	/* event_table */
#endif
);

extern Boolean _XtOnGrabList(
#if NeedFunctionPrototypes
    Widget	/* widget */,
    XtGrabRec*	/* grabList */
#endif
);

#endif /* _Event_h_ */
