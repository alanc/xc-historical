/* $XConsortium: Callback.c,v 1.29 90/12/12 14:50:30 rws Exp $ */

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

#include "IntrinsicI.h"

XtPointer XtGarbageCollection = NULL;
static String XtNinvalidCallbackList = "invalidCallbackList";
static String XtNxtAddCallback = "xtAddCallback";
static String XtNxtRemoveCallback = "xtRemoveCallback";
static String XtNxtRemoveAllCallback = "xtRemoveAllCallback";
static String XtNxtCallCallback = "xtCallCallback";

/* However it doesn't contain a final NULL record */
#define ToList(p) ((XtCallbackList) ((p)+1))

static InternalCallbackList* FetchInternalList(widget, name)
    Widget	widget;
    String	name;
{
    register XrmQuark		quark;
    register int 		n;
    register CallbackTable	offsets;

    quark = StringToQuark(name);
    offsets = (CallbackTable) 
	widget->core.widget_class->core_class.callback_private;

    for (n = (int) *(offsets++); --n >= 0; offsets++)
	if (quark == (*offsets)->xrm_name)
	    return (InternalCallbackList *) 
		((char *) widget - (*offsets)->xrm_offset - 1);
    return NULL;
}


void _XtAddCallback(callbacks, callback, closure)
    register InternalCallbackList* callbacks;
    XtCallbackProc	    callback;
    XtPointer		    closure;
{
    register XtCallbackList cl;
    register int count;
    
    count = (*callbacks) ? (*callbacks)->count : 0;

    *callbacks = (InternalCallbackList)
	XtRealloc((char *) *callbacks, sizeof(InternalCallbackRec) +
		  sizeof(XtCallbackRec) * (count + 1));

    (*callbacks)->count = count + 1;
    cl = ToList(*callbacks) + count;
    cl->callback = callback;
    cl->closure = closure;
} /* _XtAddCallback */

void _XtAddCallbackOnce(callbacks, callback, closure)
    register InternalCallbackList*callbacks;
    XtCallbackProc	    callback;
    XtPointer		    closure;
{
    register XtCallbackList cl = ToList(*callbacks);
    register int i;
    
    for (i=(*callbacks)->count; --i >= 0; cl++)
	if (cl->callback == callback && cl->closure == closure)
	    return;

    _XtAddCallback(callbacks, callback, closure);
} /* _XtAddCallbackOnce */

#if NeedFunctionPrototypes
void XtAddCallback(
    Widget	    widget,
    _Xconst char*   name,
    XtCallbackProc  callback,
    XtPointer	    closure
    )
#else
void XtAddCallback(widget, name, callback, closure)
    Widget	    widget;
    String	    name;
    XtCallbackProc  callback;
    XtPointer	    closure;
#endif
{
    InternalCallbackList *callbacks;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtAddCallback,XtCXtToolkitError,
              "Cannot find callback list in XtAddCallback",
	      (String *)NULL, (Cardinal *)NULL);
       return;
    }
    _XtAddCallback(callbacks, callback, closure);
} /* XtAddCallback */

/* ARGSUSED */
static void AddCallbacks(widget, callbacks, newcallbacks)
    Widget		    widget;
    register InternalCallbackList *callbacks;
    XtCallbackList	    newcallbacks;
{
    register int i, j;
    register XtCallbackList cl;

    i = (*callbacks) ? (*callbacks)->count : 0;
    for (j=0, cl = newcallbacks; cl->callback != NULL; cl++, j++);

    *callbacks = (InternalCallbackList) XtRealloc((char *) *callbacks,
					     sizeof(InternalCallbackRec) + 
					     sizeof(XtCallbackRec) * (i+j));

    (*callbacks)->count = i+j;
    for (cl = ToList(*callbacks) + i; --j >= 0; cl++, newcallbacks++)
	*cl = *newcallbacks;
} /* AddCallbacks */

#if NeedFunctionPrototypes
void XtAddCallbacks(
    Widget	    widget,
    _Xconst char*   name,
    XtCallbackList  xtcallbacks
    )
#else
void XtAddCallbacks(widget, name, xtcallbacks)
    Widget	    widget;
    String	    name;
    XtCallbackList  xtcallbacks;
#endif
{
    InternalCallbackList* callbacks;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtAddCallback,XtCXtToolkitError,
              "Cannot find callback list in XtAddCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
       return;
    }
    AddCallbacks(widget, callbacks, xtcallbacks);
} /* XtAddCallbacks */

void _XtRemoveCallback (callbacks, callback, closure)
    register InternalCallbackList *callbacks;
    XtCallbackProc	    callback;
    XtPointer		    closure;

{
    register int i;
    register XtCallbackList cl, ncl;

    if (*callbacks == NULL) return;

    cl = ToList(*callbacks);
    for (i=(*callbacks)->count; --i >= 0; cl++) {
	if (cl->callback == callback && cl->closure == closure) {
	    for (ncl = cl + 1; i > 0; ncl++, cl++, i--)
		*cl = *ncl;
	    if (--(*callbacks)->count)
		*callbacks = (InternalCallbackList)
		    XtRealloc((char *) *callbacks, sizeof(InternalCallbackRec)
			      + sizeof(XtCallbackRec) * (*callbacks)->count);
	    else {
		XtFree((char *) *callbacks);
		*callbacks = NULL;
	    }
	    return;
	}
    }
} /* _XtRemoveCallback */

#if NeedFunctionPrototypes
void XtRemoveCallback (
    Widget	    widget,
    _Xconst char*   name,
    XtCallbackProc  callback,
    XtPointer	    closure
    )
#else
void XtRemoveCallback (widget, name, callback, closure)
    Widget	    widget;
    String	    name;
    XtCallbackProc  callback;
    XtPointer	    closure;
#endif
{
    InternalCallbackList *callbacks;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtRemoveCallback,XtCXtToolkitError,
              "Cannot find callback list in XtRemoveCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }

    _XtRemoveCallback(callbacks, callback, closure);
} /* XtRemoveCallback */


#if NeedFunctionPrototypes
void XtRemoveCallbacks (widget, name, xtcallbacks)
    Widget	    widget;
    _Xconst char*   name;
    register XtCallbackList  xtcallbacks;
#else
void XtRemoveCallbacks (widget, name, xtcallbacks)
    Widget	    widget;
    String	    name;
    register XtCallbackList  xtcallbacks;
#endif
{
    register int i, remaining;
    register XtCallbackList cl, ncl;
    register InternalCallbackList *callbacks, old, new;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtRemoveCallback,XtCXtToolkitError,
              "Cannot find callback list in XtRemoveCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }

    old = *callbacks;
    if (old == NULL) return;

    remaining = old->count;
    for (ncl=xtcallbacks; ncl->callback != NULL; ncl++)
	for (i=old->count, cl=ToList(old); --i >= 0; cl++)
	    if (cl->callback == ncl->callback && cl->closure == ncl->closure) {
		remaining--;
		cl->callback = NULL;
		break;
	    }

    if (remaining) {
	new = (InternalCallbackList) XtMalloc(sizeof(InternalCallbackRec) +
			                   sizeof(XtCallbackRec) * remaining);
	new->count = remaining;
	ncl = ToList(new);
	for (i=old->count, cl=ToList(old); --i >= 0; cl++)
	    if (cl->callback != NULL)
		*ncl++ = *cl;
	XtFree((char *) old);
	*callbacks = new;
    } else {
	XtFree((char *) old);
	*callbacks = NULL;
    }
    
} /* XtRemoveCallbacks */


void _XtRemoveAllCallbacks (callbacks)
    InternalCallbackList *callbacks;
{
    if (*callbacks) {
	XtFree((char *) *callbacks);
	*callbacks = NULL;
    }
} /* _XtRemoveAllCallbacks */

#if NeedFunctionPrototypes
void XtRemoveAllCallbacks(widget, name)
    Widget widget;
    _Xconst char* name;
#else
void XtRemoveAllCallbacks(widget, name)
    Widget widget;
    String name;
#endif
{
    InternalCallbackList *callbacks;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtRemoveAllCallback,XtCXtToolkitError,
              "Cannot find callback list in XtRemoveAllCallbacks",
	      (String *)NULL, (Cardinal *)NULL);

	return;
    }
    _XtRemoveAllCallbacks(callbacks);
} /* XtRemoveAllCallbacks */


#define CALLBACK_CACHE_SIZE	30

void _XtCallCallbackList(widget, list, count, call_data)
    Widget		widget;
    XtCallbackList	list;
    int			count;
    XtPointer		call_data;
{
    register XtCallbackList cl;
    XtCallbackRec	 stack_cache [CALLBACK_CACHE_SIZE];
    XtCallbackList	 head;
    register int	 i;


    /* If lots of entries allocate an array, otherwise use stack_cache */
    if (count > CALLBACK_CACHE_SIZE) {
	head = (XtCallbackList) XtMalloc((unsigned)
					 (count * sizeof(XtCallbackRec)));
    } else {
	head = stack_cache;
    }

    /* Copy callback list into array to insulate us from callbacks that change
       the callback list */
    for (cl=list, i=0; i < count; cl++, i++) {
	head[i] = *cl;
    }

    /* Execute each callback in the array */
    
    for (cl = head; --i >= 0; cl++) {
	(*cl->callback) (widget, cl->closure, call_data);
    }

    /* If temporary array allocated, free it */
    if (head != stack_cache) {
	XtFree((char *)head);
    }
} /* _XtCallCallbackList */


InternalCallbackList _XtCompileCallbackList(xtcallbacks)
    XtCallbackList xtcallbacks;
{
    register int n;
    register XtCallbackList xtcl, cl;
    register InternalCallbackList callbacks;

    for (n=0, xtcl=xtcallbacks; xtcl->callback != NULL; n++, xtcl++) {};
    if (n == 0) return (InternalCallbackList) NULL;

    callbacks = (InternalCallbackList) XtMalloc(sizeof(InternalCallbackRec) +
					   sizeof(XtCallbackRec) * n);
    callbacks->count = n;
    cl = ToList(callbacks);
    while (--n >= 0)
	*(cl++) = *(xtcallbacks++);
    return(callbacks);
} /* _XtCompileCallbackList */


XtCallbackList _XtGetCallbackList(callbacks)
    InternalCallbackList callbacks;
{
    register int i;
    register XtCallbackList xtcl, cl;
    XtPointer garbage;
    XtCallbackList xtcallbacks;

    if (callbacks == NULL) {
	static XtCallbackRec emptyList[1] = { {NULL, NULL} };
	return (XtCallbackList)emptyList;
    }

    for (garbage = XtGarbageCollection; garbage != (XtPointer) NULL;
	 garbage = (XtPointer) *(XtPointer *)garbage) ;

    i = callbacks->count;
    garbage = XtMalloc(sizeof(XtCallbackRec) * (i+1) + sizeof(XtPointer));
    xtcallbacks = (XtCallbackList) ((char *)garbage + sizeof(XtPointer));
    *((XtPointer *)garbage) = NULL;

    for(cl=ToList(callbacks), xtcl=xtcallbacks; --i >= 0; cl++,xtcl++)
	*xtcl = *cl;

    xtcl->callback = (XtCallbackProc) NULL;
    xtcl->closure = NULL;
    return xtcallbacks;
}


void _XtCallCallbacks (widget, callbacks, call_data)
    Widget		widget;
    InternalCallbackList callbacks;
    XtPointer     	call_data;
{
    XtCallbackList	cl;

    if (callbacks == NULL) return;
    cl = ToList(callbacks);

    if (callbacks->count == 1) {
	(*cl->callback) (widget, cl->closure, call_data);
	return;
    }
    _XtCallCallbackList(widget, cl, callbacks->count, call_data);
}


#if NeedFunctionPrototypes
void XtCallCallbacks(
    Widget   widget,
    _Xconst char* name,
    XtPointer call_data
    )
#else
void XtCallCallbacks(widget, name, call_data)
    Widget   widget;
    String   name;
    XtPointer call_data;
#endif
{
    InternalCallbackList *callbacks;

    callbacks = FetchInternalList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       XtNinvalidCallbackList,XtNxtCallCallback,XtCXtToolkitError,
              "Cannot find callback list in XtCallCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }

    _XtCallCallbacks(widget, *callbacks, call_data);
} /* XtCallCallbacks */


#if NeedFunctionPrototypes
XtCallbackStatus XtHasCallbacks(
     Widget		widget,
     _Xconst char*	callback_name
     )
#else
XtCallbackStatus XtHasCallbacks(widget, callback_name)
     Widget		widget;
     String		callback_name;
#endif
{
    InternalCallbackList *callbacks;
    callbacks = FetchInternalList(widget, callback_name);
    if (callbacks == NULL)
	return XtCallbackNoList;
    else if (*callbacks == NULL)
	return XtCallbackHasNone;
    return XtCallbackHasSome;
} /* XtHasCallbacks */


void XtCallCallbackList(widget, callbacks, call_data)
    Widget widget;
    XtCallbackList callbacks;
    XtPointer call_data;
{
    register int n;
    register InternalCallbackList cl;

    if (callbacks == NULL)
	return;
    cl = (InternalCallbackList) callbacks;
    n = cl->count;
    callbacks = ToList(cl);
    if (n == 1) {
	(*callbacks->callback) (widget, callbacks->closure, call_data);
	return;
    }
    _XtCallCallbackList(widget, callbacks, n, call_data);
}
