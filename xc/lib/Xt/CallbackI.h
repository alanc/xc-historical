/* $XConsortium: CallbackI.h,v 1.11 90/08/29 13:09:40 swick Exp $ */
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

/****************************************************************
 *
 * Callbacks
 *
 ****************************************************************/

typedef XrmResource **CallbackTable;

typedef struct internalCallbackRec {
    int	count;
    /* XtCallbackList */
} InternalCallbackRec, *InternalCallbackList;

extern XtPointer XtGarbageCollection;


extern void _XtAddCallback(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer 			/* closure */
#endif
);

extern void _XtAddCallbackOnce(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer 			/* closure */
#endif
);

extern void _XtCallCallbacks(
#if NeedFunctionPrototypes
    Widget			/* widget */,
    InternalCallbackList	/* callbacks */,
    XtPointer 			/* callData */
#endif
);

extern InternalCallbackList _XtCompileCallbackList(
#if NeedFunctionPrototypes
    XtCallbackList		/* xtcallbacks */
#endif
);

extern XtCallbackList _XtGetCallbackList(
#if NeedFunctionPrototypes
    InternalCallbackList	/* list */
#endif
);

extern void _XtRemoveAllCallbacks(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */
#endif
);

extern void _XtRemoveCallback(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer			/* closure */
#endif
);
