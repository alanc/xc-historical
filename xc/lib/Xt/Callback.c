#ifndef lint
static char Xrcsid[] = "$XConsortium: Callback.c,v 1.8 88/09/06 10:01:10 swick Exp $";
/* $oHeader: Callback.c,v 1.4 88/09/01 11:08:37 asente Exp $ */
#endif lint

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

extern CallbackList _CompileCallbackList();


typedef struct _CallbackRec {
    CallbackList  next;
    Widget	    widget;
    XtCallbackProc  callback;
    Opaque	    closure;
} CallbackRec;

static CallbackList *FetchCallbackList (widget, name)
    Widget  widget;
    String  name;
{
    register _XtOffsetList  offsets;
    register XrmQuark       quark;

    quark = StringToQuark(name);
    for (offsets = widget->core.widget_class->core_class.callback_private;
	 offsets != NULL;
	 offsets = offsets->next) {
	if (quark == offsets->name) {
	    return((CallbackList *) ((char *) widget - offsets->offset - 1));
	}
    }
    return(NULL);
} /* FetchCallbackList */


void _XtAddCallback(widget, callbacks, callback, closure)
    Widget		    widget;
    register CallbackList   *callbacks;
    XtCallbackProc	    callback;
    Opaque		    closure;
{
    register CallbackRec *new;

    new = XtNew(CallbackRec);
    new->next = NULL;
    new->widget = widget;
    new->closure = closure;
    new->callback = callback; 

    for ( ; *callbacks != NULL; callbacks = &(*callbacks)->next) {};
    *callbacks = new;
} /* _XtAddCallback */

void XtAddCallback(widget, name, callback, closure)
    Widget	    widget;
    String	    name;
    XtCallbackProc  callback;
    Opaque	    closure;
{
    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget,name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtAddCallback","XtToolkitError",
              "Cannot find callback list in XtAddCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
       return;
    }
    _XtAddCallback(widget, callbacks, callback, closure);
} /* XtAddCallbacks */

/* ARGSUSED */
static void AddCallbacks(widget, callbacks, newcallbacks)
    Widget		    widget;
    register CallbackList   *callbacks;
    CallbackList	    newcallbacks;
{
    for ( ; *callbacks != NULL; callbacks = &(*callbacks)->next) {};
    *callbacks = newcallbacks;
} /* AddCallback */

void XtAddCallbacks(widget, name, xtcallbacks)
    Widget	    widget;
    String	    name;
    XtCallbackList     xtcallbacks;
{
    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtAddCallback","XtToolkitError",
              "Cannot find callback list in XtAddCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
       return;
    }
    AddCallbacks(widget, callbacks, _CompileCallbackList(widget, xtcallbacks));
} /* XtAddCallbacks */

void RemoveCallback (widget, callbacks, callback, closure)
    Widget		    widget;
    register CallbackList   *callbacks;
    XtCallbackProc	    callback;
    Opaque		    closure;

{
    register CallbackList cl;

    for (cl = *callbacks; cl != NULL; (cl = *(callbacks = &cl->next))) {
	if ((cl->widget == widget) && (cl->closure == closure)
                            && (cl->callback == callback)) {
	    *callbacks = cl->next;
	    XtFree ((char *)cl);
	    return;
	}
    }
} /* RemoveCallback */

void XtRemoveCallback (widget, name, callback, closure)
    Widget	    widget;
    String	    name;
    XtCallbackProc  callback;
    Opaque	    closure;
{

    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtRemoveCallback","XtToolkitError",
              "Cannot find callback list in XtRemoveCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }
    RemoveCallback(widget, callbacks, callback, closure);
} /* XtRemoveCallback */


void XtRemoveCallbacks (widget, name, xtcallbacks)
    Widget	    widget;
    String	    name;
    register XtCallbackList  xtcallbacks;
{

    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtRemoveCallback","XtToolkitError",
              "Cannot find callback list in XtRemoveCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }

    for (; xtcallbacks->callback != NULL; xtcallbacks++) {
	RemoveCallback(
	    widget, callbacks, xtcallbacks->callback,
	    (Opaque) xtcallbacks->closure);
    }
} /* XtRemoveCallbacks */


void _XtRemoveAllCallbacks (callbacks)
    CallbackList *callbacks;
{
    register CallbackList cl, next;
    
    cl = *callbacks;
    while (cl != NULL) {
	next = cl->next;
	XtFree((char *)cl);
	cl = next;
    }
    (*callbacks) = NULL;
} /* _XtRemoveAllCallbacks */

void XtRemoveAllCallbacks(widget, name)
    Widget widget;
    String name;
{
    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtRemoveAllCallback","XtToolkitError",
              "Cannot find callback list in XtRemoveAllCallbacks",
	      (String *)NULL, (Cardinal *)NULL);

	return;
    }
    _XtRemoveAllCallbacks(callbacks);
} /* XtRemoveAllCallbacks */



#define CALLBACK_CACHE_SIZE	30

void _XtCallCallbacks (callbacks, call_data)
    CallbackList *callbacks;
    Opaque       call_data;
{
    register CallbackRec *cl;
    CallbackRec		 stack_cache [CALLBACK_CACHE_SIZE];
    CallbackList	 head;
    register Cardinal    i;

    if ((*callbacks) == NULL ) return;

    /* Get length of callback list */
    for (i = 0, cl = *callbacks; cl != NULL; i++, cl = cl->next) {};

    /* If lots of entries allocate an array, otherwise use stack_cache */
    if (i > CALLBACK_CACHE_SIZE) {
	head = (CallbackList) XtMalloc((unsigned) (i * sizeof(CallbackRec)));
    } else {
	head = stack_cache;
    }

    /* Copy callback list into array to insulate us from callbacks that change
       the callback list */
    for (i = 0, cl = *callbacks; cl != NULL; i++, cl = cl->next) {
	head[i] = *cl;
    }

    /* Execute each callback in the array */
    for (cl = head; i != 0; cl++, i--) {
	(*(cl->callback)) (cl->widget, cl->closure, call_data);
    }

    /* If temporary array allocated, free it */
    if (head != stack_cache) {
	XtFree((char *)head);
    }
} /* _XtCallCallbacks */


CallbackList _CompileCallbackList(widget, xtcallbacks)
    Widget		    widget;
    register XtCallbackList xtcallbacks;
{
    /* Turn a public XtCallbackList into a private CallbackList */

    register CallbackList   new, *pLast;
    CallbackList	    head;

    pLast = &head;
    for (; xtcallbacks->callback != NULL; xtcallbacks++) {
	new		= XtNew(CallbackRec);
	*pLast		= new;
	pLast		= &(new->next);
	new->widget     = widget;
	new->callback   = xtcallbacks->callback;
	new->closure    = (Opaque) xtcallbacks->closure;
    };
    *pLast = NULL;

    return(head);
} /* _CompileCallbackList */

void XtCallCallbacks(widget, name, call_data)
    Widget   widget;
    String   name;
    Opaque  call_data;
{
    CallbackList *callbacks;

    callbacks = FetchCallbackList(widget, name);
    if (callbacks == NULL) {
       XtAppWarningMsg(XtWidgetToApplicationContext(widget),
	       "invalidCallbackList","xtCallCallback","XtToolkitError",
              "Cannot find callback list in XtCallCallbacks",
	      (String *)NULL, (Cardinal *)NULL);
	return;
    }
    _XtCallCallbacks(callbacks, call_data);
} /* XtCallCallbacks */

/* ||| What is this doing here? */
extern XtCallbackStatus XtHasCallbacks(widget, callback_name)
     Widget		widget;
     String		callback_name;
{
    CallbackList *callbacks;
    callbacks = FetchCallbackList(widget, callback_name);
    if (callbacks == NULL) {
	return XtCallbackNoList;
    }    
    if (*callbacks == NULL) return XtCallbackHasNone;
    return XtCallbackHasSome;
} /* XtHasCallbacks */
