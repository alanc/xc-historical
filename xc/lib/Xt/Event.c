#ifndef lint
static char rcsid[] = "$Header: Event.c,v 1.71 88/04/22 16:30:51 swick Exp $";
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "IntrinsicI.h"
#include "Event.h"
#include <X11/StringDefs.h>
#include <X11/Shell.h>
extern void bzero();

static GrabRec *grabList;
static GrabRec *freeGrabs;
static Boolean OnlyKeyboardGrabs;
static Boolean FocusTraceGood;


EventMask _XtBuildEventMask(widget)
    Widget widget;
{
    _XtEventTable ev;
    EventMask	mask = 0;

    for (ev = widget->core.event_table; ev != NULL; ev = ev->next)
	if (ev->select) mask |= ev->mask;
    if (widget->core.widget_class->core_class.expose != NULL)
	mask |= ExposureMask;
    if (widget->core.widget_class->core_class.visible_interest) 
	mask |= VisibilityChangeMask;

    return mask;
}

static void RemoveEventHandler(widget, eventMask, other, proc, closure, raw)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
    Boolean	raw;
{
    XtEventRec *p, **pp;
    EventMask oldMask = _XtBuildEventMask(widget);

    pp = &widget->core.event_table;
    p = *pp;

    /* find it */
    while (p != NULL && (p->proc != proc || p->closure != closure)) {
	pp = &p->next;
	p = *pp;
    }
    if (p == NULL) return; /* couldn't find it */
    if (raw) p->raw = FALSE; else p->select = FALSE;
    if (p->raw || p->select) return;

    /* un-register it */
    p->mask &= ~eventMask;
    if (other) p->non_filter = FALSE;

    if (p->mask == 0 && !p->non_filter) {
	/* delete it entirely */
	*pp = p->next;
	XtFree((char *)p);
    }

    /* reset select mask if realized */
    if (XtIsRealized(widget)) {
	EventMask mask = _XtBuildEventMask(widget);

	if (oldMask != mask)
	    XSelectInput(XtDisplay(widget), XtWindow(widget), mask);
    }
}

static void AddEventHandler(widget, eventMask, other, proc, closure, raw)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
    Boolean	raw;
{
   register XtEventRec *p,**pp;
   EventMask oldMask;

   if (eventMask == 0 && other == FALSE) return;

   if (XtIsRealized(widget) && ! raw) oldMask = _XtBuildEventMask(widget);

   pp = & widget->core.event_table;
   p = *pp;
   while (p != NULL && (p->proc != proc || p->closure != closure)) {
         pp = &p->next;
         p = *pp;
   }

   if (p == NULL) {
	/* new proc to add to list */
	p = XtNew(XtEventRec);
	p->proc = proc;
	p->closure = closure;
	p->mask = eventMask;
	p->non_filter = other;
	p->select = ! raw;
	p->raw = raw;

	p->next = widget->core.event_table;
	widget->core.event_table = p;

    } else {
	/* update existing proc */
	p->mask |= eventMask;
	p->non_filter = p->non_filter || other;
	p->select |= ! raw;
	p->raw |= raw;
    }

    if (XtIsRealized(widget) && ! raw) {
	EventMask mask = _XtBuildEventMask(widget);

	if (oldMask != mask)
	    XSelectInput(XtDisplay(widget), XtWindow(widget), mask);
    }

}


void XtRemoveEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, FALSE);
}


void XtAddEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
{
    AddEventHandler(widget, eventMask, other, proc, closure, FALSE);
}


void XtRemoveRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, TRUE);
}


void XtAddRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
{
    AddEventHandler(widget, eventMask, other, proc, closure, FALSE);
}


typedef struct _HashRec *HashPtr;

typedef struct _HashRec {
    Display	*display;
    Window	window;
    Widget	widget;
    HashPtr	next;
} HashRec;

typedef struct {
    unsigned int	size;
    unsigned int	count;
    HashPtr		entries[1];
} HashTableRec, *HashTable;

static HashTable table = NULL;

static void ExpandTable();

void _XtRegisterWindow(window, widget)
    Window window;
    Widget widget;
{
    register HashPtr hp, *hpp;

    if ((table->count + (table->count / 4)) >= table->size) ExpandTable();

    hpp = &table->entries[(unsigned int)window & (table->size-1)];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window) {
	    if (hp->widget != hp->widget)
		XtWarning("Attempt to change already registered window.\n");
	    return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }

    hp = *hpp = XtNew(HashRec);
    hp->window = window;
    hp->widget = widget;
    hp->next = NULL;
    table->count++;
}


void _XtUnregisterWindow(window, widget)
    Window window;
    Widget widget;
{
    HashPtr hp, *hpp;

    hpp = &table->entries[(unsigned int)window  & (table->size-1)];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window) {
	    if (hp->widget != widget) {
		XtWarning("Unregister-window does not match widget.\n");
                return;
                }
             else /* found entry to delete */
                  (*hpp) = hp->next;
                  XtFree((char*)hp);
                  table->count--;
                  return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }
    
}

static void ExpandTable()
{
    HashTable	oldTable = table;
    unsigned int i;

    i = oldTable->size * 2;
    table = (HashTable) XtMalloc(
	(unsigned) sizeof(HashTableRec)+i*sizeof(HashRec));
    table->size = i;
    table->count = oldTable->count;
    for (i = 0; i<oldTable->size; i++) {
	HashPtr hp;
	hp = oldTable->entries[i];
	while (hp != NULL) {
	    HashPtr temp = hp;
	    _XtRegisterWindow(hp->window, hp->widget);
	    hp = hp->next;
	    XtFree((char *) temp);
	}
    }
    XtFree((char *)oldTable);
}


/*ARGSUSED*/
Widget XtWindowToWidget(display, window)
    Display *display;
    Window window;
{
    register HashPtr hp;

/* display is ignored for now; will be used when multiple displays
   are supported */

    for (
        hp = table->entries[(unsigned int)window & (table->size-1)];
        hp != NULL;
	hp = hp->next)
	if (hp->window == window) return hp->widget;

    return NULL;
}

static void InitializeHash()
{
    int size = sizeof(HashTableRec)+1024*sizeof(HashPtr);

    table = (HashTable) XtMalloc((unsigned) size);
    bzero((char *) table, size);

    table->size = 1024;
    table->count = 0;
}


XtAddExposureToRegion(event, region)
    XEvent   *event;
    Region   region;
{
    XRectangle rect;

    switch (event->type) {
	case Expose:
		rect.x = event->xexpose.x;
		rect.y = event->xexpose.y;
		rect.width = event->xexpose.width;
		rect.height = event->xexpose.height;
		break;
	case GraphicsExpose:
		rect.x = event->xgraphicsexpose.x;
		rect.y = event->xgraphicsexpose.y;
		rect.width = event->xgraphicsexpose.width;
		rect.height = event->xgraphicsexpose.height;
		break;
	default:
		return;
    }

    XUnionRectWithRegion(&rect, region, region);
}


/* %%% Multiple display support will require keeping an exposeRegion
 *     for each open display
 */

static Region nullRegion;
static Region exposeRegion;

static void DispatchEvent(event, widget, mask)
    register XEvent    *event;
    Widget    widget;
    unsigned long mask;
{
    XtEventRec *p;   
    XtEventHandler proc[100];
    caddr_t closure[100];
    int numprocs, i;
    XEvent nextEvent;

    if (mask == ExposureMask) {
	XtExposeProc expose = widget->core.widget_class->core_class.expose;
	if (expose != NULL) {
	    if (!widget->core.widget_class->core_class.compress_exposure) {
		(*expose)(widget, event, (Region)NULL);
	    }
	    else {
		XtAddExposureToRegion(event, exposeRegion);
		if (event->xexpose.count == 0) {
		    /* Patch event to have the new bounding box.  Unless
		       someone's goofed, it can only be an Expose event */
		    XRectangle rect;
		    XClipBox(exposeRegion, &rect);
		    event->xexpose.x = rect.x;
		    event->xexpose.y = rect.y;
		    event->xexpose.width = rect.width;
		    event->xexpose.height = rect.height;
		    (*expose)(widget, event, exposeRegion);
		    XIntersectRegion( nullRegion, exposeRegion, exposeRegion );
		}
	    }
	}
    }

    if (mask == EnterWindowMask &&
	    widget->core.widget_class->core_class.compress_enterleave) {
	if (XPending(event->xcrossing.display)) {
	    XPeekEvent(event->xcrossing.display, &nextEvent);
	    if (nextEvent.type == LeaveNotify &&
		    event->xcrossing.window == nextEvent.xcrossing.window &&
		    event->xcrossing.subwindow == nextEvent.xcrossing.subwindow) {
		/* skip the enter/leave pair */
		XNextEvent(event->xcrossing.display, &nextEvent);
		return;
	    }
	}
    }

    if (event->type == MotionNotify &&
	    widget->core.widget_class->core_class.compress_motion) {
	while (XPending(event->xmotion.display)) {
	    XPeekEvent(event->xmotion.display, &nextEvent);
	    if (nextEvent.type == MotionNotify &&
		    event->xmotion.window == nextEvent.xmotion.window &&
		    event->xmotion.subwindow == nextEvent.xmotion.subwindow) {
		/* replace the current event with the next one */
		XNextEvent(event->xmotion.display, event);
	    } else break;
	}
    }

    if (mask == VisibilityChangeMask &&
            ! widget->core.widget_class->core_class.visible_interest) return;

    /* Have to copy the procs into an array, because calling one of them */
    /* might call XtRemoveEventHandler, which would break our linked list.*/

    numprocs = 0;

    for (p=widget->core.event_table; p != NULL; p = p->next) {
	if ((mask & p->mask) != 0 || (mask == 0 && p->non_filter)) {
	    proc[numprocs] = p->proc;
	    closure[numprocs++] = p->closure;
	}
    }

    for (i=0 ; i<numprocs ; i++) (*(proc[i]))(widget, closure[i], event);
}


typedef enum {pass, ignore, remap} GrabType;

static void ConvertTypeToMask (eventType, mask, grabType)
    int		eventType;
    EventMask   *mask;
    GrabType    *grabType;
{

static struct {
    EventMask   mask;
    GrabType    grabType;
} masks[] = {
    {0,				pass},      /* shouldn't see 0  */
    {0,				pass},      /* shouldn't see 1  */
    {KeyPressMask,		remap},     /* KeyPress		*/
    {KeyReleaseMask,		remap},     /* KeyRelease       */
    {ButtonPressMask,		remap},     /* ButtonPress      */
    {ButtonReleaseMask,		remap},     /* ButtonRelease    */
    {PointerMotionMask
     | Button1MotionMask
     | Button2MotionMask
     | Button3MotionMask
     | Button4MotionMask
     | Button5MotionMask
     | ButtonMotionMask,	ignore},    /* MotionNotify	*/
    {EnterWindowMask,		ignore},    /* EnterNotify	*/
    {LeaveWindowMask,		ignore},    /* LeaveNotify	*/
    {FocusChangeMask,		pass},      /* FocusIn		*/
    {FocusChangeMask,		pass},      /* FocusOut		*/
    {KeymapStateMask,		pass},      /* KeymapNotify	*/
    {ExposureMask,		pass},      /* Expose		*/
    {0,				pass},      /* GraphicsExpose   */
    {0,				pass},      /* NoExpose		*/
    {VisibilityChangeMask,      pass},      /* VisibilityNotify */
    {SubstructureNotifyMask,    pass},      /* CreateNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* DestroyNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* UnmapNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* MapNotify	*/
    {SubstructureRedirectMask,  pass},      /* MapRequest	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* ReparentNotify   */
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* ConfigureNotify  */
    {SubstructureRedirectMask,  pass},      /* ConfigureRequest */
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* GravityNotify	*/
    {ResizeRedirectMask,	pass},      /* ResizeRequest	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* CirculateNotify	*/
    {SubstructureRedirectMask,  pass},      /* CirculateRequest */
    {PropertyChangeMask,	pass},      /* PropertyNotify   */
    {0,				pass},      /* SelectionClear   */
    {0,				pass},      /* SelectionRequest */
    {0,				pass},      /* SelectionNotify  */
    {ColormapChangeMask,	pass},      /* ColormapNotify   */
    {0,				pass},      /* ClientMessage	*/
    {0,				pass},      /* MappingNotify ???*/
  };

    eventType &= 0x7f;	/* Events sent with XSendEvent have high bit set. */
    (*mask)      = masks[eventType].mask;
    (*grabType)  = masks[eventType].grabType;
};


static Boolean OnGrabList (widget)
    register Widget widget;

{
    register GrabRec* gl;
    for (; widget != NULL; widget = (Widget)widget->core.parent)
	for (gl = grabList; gl != NULL; gl = gl->next) {
	    if (gl->widget == widget) return True;
	    if (gl->exclusive) break;
	}
    return False;
}


static Widget FindKeyboardFocus(widget)
    Widget widget;
{
    /* If a modal cascade exists, this routine
       assumes that widget is in the active subset.
     */
    static Widget focus_trace_widget, focus_target_widget;
    register GrabRec* gl;
#define FOCUS_TRACE_SIZE 100
    GrabRec* focus_trace[FOCUS_TRACE_SIZE];
    int i = 0, num_focus_entries;
    enum {non_exclusive, first_pass, dont_check} exclusive = non_exclusive;
    register Widget w, focus_widget = widget;

    if (FocusTraceGood && widget == focus_trace_widget)
	return focus_target_widget;
    focus_trace_widget = widget;
    for (num_focus_entries = 0, gl = grabList; gl != NULL; gl = gl->next) {
	if (gl->keyboard_focus != NULL) num_focus_entries++;
    }
    if (num_focus_entries == 0) return focus_target_widget = widget;
    /* assert: a grab for a widget always occurs on the list before
     * a keyboard focus entry for the same widget
     */
    for (w = widget; w != NULL; w = XtParent(w)) {
	for (gl = grabList; gl != NULL; gl = gl->next) {
	    if (gl->widget == w) {
		if (gl->keyboard_focus != NULL) {
		    if (i == FOCUS_TRACE_SIZE)
			XtError("FindKeyboardFocus: nesting depth exceeded.");
		    focus_trace[i++] = gl;
		    break;
		}
		else if (gl->exclusive) {
		    if (exclusive == dont_check)
			break;
		    else
			exclusive = first_pass;
		}
	    }
	    else if (gl->exclusive && exclusive == dont_check)
		break;
	}
	if (i == num_focus_entries) break;
	if (exclusive == first_pass) exclusive = dont_check;
    }
    if (i > 0) {
	focus_widget = focus_trace[--i]->keyboard_focus;
	while (i > 0) {
	    if (focus_trace[--i]->widget == focus_widget)
		focus_widget = focus_trace[i]->keyboard_focus;
	}
    }
    /* replace with focus widget iff source widget
     * is not a descendant of the focus widget
     */
    for (w = widget; w != NULL && w != focus_widget; w = XtParent(w));
    FocusTraceGood = True;
    return focus_target_widget = (w == focus_widget) ? widget : focus_widget;
}


void XtDispatchEvent (event)
    XEvent  *event;
{
    register    Widget widget;
    EventMask   mask;
    GrabType    grabType;
    register    GrabList gl;

#define ShouldDispatch \
    (widget->core.sensitive && widget->core.ancestor_sensitive)

    widget = XtWindowToWidget (event->xany.display, event->xany.window);

    /* Lint complains about &grabType not matching the declaration.
       Don't bother trying to fix it, it won't work */

    ConvertTypeToMask(event->xany.type, &mask, &grabType);

    if (widget == NULL) {
	if (grabType != remap) return;
	/* event occurred in a non-widget window, but we've promised
	   also to dispatch it to the nearest spring_loaded widget */
	for (gl = grabList; gl != NULL; gl = gl->next) {
	    if (gl->spring_loaded) {
		DispatchEvent(event, gl->widget, mask);
		return;
	    }
	}
    }

    if (grabType == pass) {
	DispatchEvent(event, widget, mask);

    } else if (grabList == NULL) {
	if ShouldDispatch {
	    DispatchEvent(event, widget, mask);
	}

    } else if (grabType == remap) {
        if ((mask & (ButtonPressMask|ButtonReleaseMask) && OnlyKeyboardGrabs)
	    || OnGrabList(widget)) {
	    if (mask & (KeyPressMask|KeyReleaseMask))
		widget = FindKeyboardFocus(widget);
	    if ShouldDispatch {
		DispatchEvent(event, widget, mask);
	    }
	}
	/* Also dispatch to nearest accessible spring_loaded */
        /* Dispatching the event may invoke a proc that popdowns all
           the popups and un-registers all the grabs from the grabList,
           so check for something still on the grabList. */
 	gl = grabList;
	while (gl!= NULL) {
	    if (gl->spring_loaded) {
		if (gl->widget != widget) {
		    DispatchEvent(event, gl->widget, mask);
		}
		break;
	    }
	    if (gl->exclusive) {
		break;
	    }
	    gl = gl->next;
	} 
    } else if (OnlyKeyboardGrabs || OnGrabList(widget)) {
	if ShouldDispatch {
	    DispatchEvent(event, widget, mask);
	}
    }
}

static Boolean RemoveGrab();

/* ARGSUSED */
static void GrabDestroyCallback(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;		/* Widget or NULL */
    caddr_t call_data;
{
    /* Remove widget from grab list if it destroyed */
    if (closure != NULL)	/* removing a keyboard grab on some parent? */
	(void)RemoveGrab( (Widget)closure, True );
    else
	(void)RemoveGrab( widget, False );
}

static void AddGrab(widget, exclusive, spring_loaded, keyboard_focus)
    Widget  widget;
    Boolean exclusive;
    Boolean spring_loaded;
    Widget  keyboard_focus;
{
    register GrabList    gl;

    if (freeGrabs == NULL)
	gl	      = /* XtNew(GrabRec) */
			(GrabRec*)Xpermalloc((unsigned int)sizeof(GrabRec));
    else {
	gl = freeGrabs;
	freeGrabs = freeGrabs->next;
    }
    gl->next	      = grabList;
    grabList	      = gl;
    gl->widget        = widget;
    gl->exclusive     = exclusive;
    gl->spring_loaded = spring_loaded;
    gl->keyboard_focus= keyboard_focus;

    if (keyboard_focus == (Widget)NULL)
	OnlyKeyboardGrabs = False;
    else if (gl->exclusive)
	FocusTraceGood = False;

    XtAddCallback(
		  (keyboard_focus != NULL) ? keyboard_focus : widget,
		  XtNdestroyCallback, GrabDestroyCallback,
		  (caddr_t)(keyboard_focus != NULL) ? widget : NULL
		 );
}

/* add a grab record to the list, or replace the focus widget in an
   existing grab record.  Returns True if the action was not a no-op.
 */
static Boolean InsertKeyboardGrab(widget, keyboard_focus)
    Widget  widget;
    Widget  keyboard_focus;
{
    register GrabRec *gl;
    register Widget w;
    GrabRec* ge;
    Boolean found = False;

    if (grabList == NULL) {
	AddGrab( widget, False, False, keyboard_focus );
	return True;
    }
    /* look for a keyboard grab entry for the same parent; if none,
       reposition this entry after any other entries for the same tree */
    for (w = widget; w != NULL && !found; w = w->core.parent) {
	for (gl = grabList; gl != NULL; gl = gl->next)
	    if (gl->widget == w) {
		found = True;
		break;
	    }
    }
    if (found)
	if (gl->widget == widget && gl->keyboard_focus != NULL) 
	    if (gl->keyboard_focus == keyboard_focus)
		return False;
	    else {
		SendFocusNotify(gl->keyboard_focus, FocusOut);
		gl->keyboard_focus = keyboard_focus;
	    }
	else {
	    AddGrab( widget, False, False, keyboard_focus );
	    ge = grabList;
	    grabList = grabList->next;
	    ge->next = gl->next;
	    gl->next = ge;
	}
    else { /* insert a new grab at end of list */
	AddGrab( widget, False, False, keyboard_focus );
	if (grabList->next != NULL) {
	    ge = grabList;
	    grabList = grabList->next;
	    ge->next = NULL;
	    for (gl = grabList; gl->next != NULL; gl = gl->next);
	    gl->next = ge;
	}
    }
    return True;
}

void XtAddGrab(widget, exclusive, spring_loaded)
    Widget  widget;
    Boolean exclusive;
    Boolean spring_loaded;
{
    if (spring_loaded && !exclusive) {
	XtWarning("XtAddGrab requires exclusive grab if spring_loaded is TRUE");
	exclusive = TRUE;
    }

    AddGrab(widget, exclusive, spring_loaded, (Widget)NULL);
}


static Boolean RemoveGrab(widget, keyboard_focus)
    Widget widget;
    Boolean keyboard_focus;
    /* returns False if no grab entry was found, True otherwise */
{
    register GrabList gl;
    register GrabList* prev;
    Widget focus_widget;

    OnlyKeyboardGrabs = True;
    for (prev = &grabList, gl = grabList; gl != NULL; gl = gl->next) {
	if (gl->widget == widget)
	    if (keyboard_focus == (gl->keyboard_focus != NULL))
		break;
	if (gl->keyboard_focus == (Widget)NULL) OnlyKeyboardGrabs = False;
	prev = &gl->next;
    }

    if (gl == NULL) {
	if (!keyboard_focus)
	    XtWarning("XtRemoveGrab asked to remove a widget not on the grab list");
	return False;
    }

    if (!keyboard_focus) {	/* remove all non-kbd grabs up to this'n */
	Boolean done;
	OnlyKeyboardGrabs = True;
	for (prev = &grabList, gl = grabList, done = False; !done; gl = *prev){
	    if (gl->keyboard_focus == (Widget)NULL) {
		*prev = gl->next;
		gl->next = freeGrabs;
		freeGrabs = gl;
		if (gl->widget == widget) done = True;
	    }
	    else
		prev = &gl->next;
	}
	if (freeGrabs->exclusive) FocusTraceGood = False;
    }
    else /* if keyboard_focus */ {
	focus_widget = gl->keyboard_focus;
	*prev = gl->next;
	gl->next = freeGrabs;
	freeGrabs = gl;
	gl = *prev;
    }
    while (gl && OnlyKeyboardGrabs) {
	if (gl->keyboard_focus == (Widget)NULL)
	    OnlyKeyboardGrabs = False;
	gl = gl->next;
    }
    XtRemoveCallback(
		     keyboard_focus ? focus_widget : widget,
		     XtNdestroyCallback, GrabDestroyCallback,
		     (caddr_t)(keyboard_focus ? widget : NULL)
		    );
    return True;
}


void XtRemoveGrab(widget)
    Widget  widget;
{
    (void)RemoveGrab(widget, False);
}


void XtMainLoop()
{
    XEvent event;

    for (;;) {
    	XtNextEvent(&event);
	XtDispatchEvent(&event);
    }
}


void _XtEventInitialize()
{
    grabList = NULL;
    freeGrabs = NULL;
    DestroyList = NULL;
    OnlyKeyboardGrabs = True;
    FocusTraceGood = False;
    nullRegion = XCreateRegion();
    exposeRegion = XCreateRegion();
    InitializeHash();
}


/* ARGSUSED */
static void ForwardEvent(ancestor, client_data, event)
    Widget ancestor;
    caddr_t client_data;
    XEvent *event;
{
    /* this shouldn't have been necessary, as the keyboard grab will cause
       needed, however, a protocol bug causes us to believe we lost the
       focus during a grab */
    EventMask mask;
    GrabType grabType;
    register Widget widget = FindKeyboardFocus( (Widget)client_data );

    if ShouldDispatch {
	ConvertTypeToMask(event->xany.type, &mask, &grabType);
	DispatchEvent(event, widget, mask);
    }
}


/* ARGSUSED */
static void HandleFocus(widget, client_data, event)
    Widget widget;
    caddr_t client_data;	/* child who wants focus */
    XEvent *event;
{
    enum {add, remove} which;
    Widget descendant = (Widget)client_data;
    switch( event->type ) {
      case EnterNotify:
      case LeaveNotify:
		if (!event->xcrossing.focus ||
/* a protocol bug causes us to not receive notification that we've lost
   the focus when the pointer leaves during a grab
		    event->xcrossing.mode != NotifyNormal ||
*/
		    event->xcrossing.detail == NotifyInferior)
		    return;
		if (event->type == EnterNotify) which = add;
		else				which = remove;
		break;
      case FocusIn:
      case FocusOut:
		if ((event->xfocus.mode != NotifyNormal &&
		     event->xfocus.mode != NotifyWhileGrabbed) ||
		    event->xfocus.detail == NotifyInferior)
		    return;
		if (event->type == FocusIn) which = add;
		else			    which = remove;
    }
    if (which == add)
	InsertKeyboardGrab( widget, descendant );
    else
	(void)RemoveGrab( widget, True );

    SendFocusNotify( descendant, (which == add) ? FocusIn : FocusOut );
}


static void AddForwardingHandler(widget, descendant)
    Widget widget, descendant;
{
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    EventMask eventMask;
    register XtEventRec* p = widget->core.event_table;
    Boolean might_have_focus = False;
    Boolean had_focus_already = False;
    register XtEventHandler proc = ForwardEvent; /* compiler bug */

    /* %%%
       Until we implement a mechanism for propagating keyboard event
       interest to all ancestors for which the descendant may be the
       focus target, the following optimization requires
       XtSetKeyboardFocus calls to be executed from the inside out.
     */
    eventMask = _XtBuildEventMask(descendant);
    eventMask &= KeyPressMask | KeyReleaseMask;

    while (p != NULL && p->proc != proc) p = p->next;
    if (p != NULL) {
	might_have_focus = True;
	if (p->mask == eventMask)
	    p->closure = (caddr_t)descendant;
	else {
	    XtRemoveEventHandler(widget, XtAllEvents, False,
				 ForwardEvent, p->closure);
	    p = NULL;
	}
    }
    if (p == NULL && eventMask != 0) {
	XtAddEventHandler
	    (
	     widget,
	     eventMask,
	     False,
	     ForwardEvent,
	     (caddr_t)descendant
	    );
    }
    if (might_have_focus) {
	/* optimization to avoid QueryPointer */
	register GrabRec *gl;
	for (gl = grabList; gl != NULL; gl = gl->next) {
	    if (gl->widget == widget && gl->keyboard_focus != NULL) {
		had_focus_already = True;
		break;
	    }
	}
    }
    if (!had_focus_already
	&& eventMask != 0
	&& (widget != XtParent(descendant) ||
	    !XtIsSubclass(widget, shellWidgetClass))) {
	/* is the pointer already inside? */
	XQueryPointer( XtDisplay(widget), XtWindow(widget),
		       &root, &child, &root_x, &root_y,
		       &win_x, &win_y, &mask );
	if (win_x >= 0 && win_x < widget->core.width &&
	    win_y >= 0 && win_y < widget->core.height) {
	    had_focus_already = True;
	}
    }
    if (had_focus_already) {
	if (InsertKeyboardGrab( widget, descendant ))
	    SendFocusNotify( descendant, FocusIn );
    }
}

/* ARGSUSED */
static void QueryEventMask(widget, client_data, event)
    Widget widget;		/* child who gets focus */
    caddr_t client_data;	/* ancestor giving it */
    XEvent *event;
{
    if (event->type == MapNotify) {
	/* make sure ancestor still wants focus set here */
	register XtEventRec* p = ((Widget)client_data)->core.event_table;
	register XtEventHandler proc = HandleFocus; /* compiler bug */
	while (p != NULL && p->proc != proc) p = p->next;
	if (p != NULL && p->closure == (caddr_t)widget)
	    AddForwardingHandler((Widget)client_data, widget);
	XtRemoveEventHandler( widget, XtAllEvents, False,
			      QueryEventMask, client_data );
    }
}


void XtSetKeyboardFocus(widget, descendant)
    Widget widget;
    Widget descendant;
{
    if (descendant == (Widget)None) {
/*	XtRemoveEventHandler(widget, XtAllEvents, True, HandleFocus, NULL);
	XtRemoveEventHandler(widget, XtAllEvents, True, ForwardEvent, NULL);
*/
	register XtEventRec* p;
	register XtEventHandler proc;
	p = widget->core.event_table;
	proc = HandleFocus;	/* compiler bug */
	while (p != NULL && p->proc != proc) p = p->next;
	if (p != NULL) {
	    descendant = (Widget)p->closure;
	    XtRemoveEventHandler(widget, XtAllEvents, False,
				 HandleFocus, p->closure);
	}
	p = widget->core.event_table;
	proc = ForwardEvent;	/* compiler bug */
	while (p != NULL && p->proc != proc) p = p->next;
	if (p != NULL) {
	    XtRemoveEventHandler(widget, XtAllEvents, False,
				 ForwardEvent, p->closure);
	}
	if (RemoveGrab( widget, True ) && descendant != (Widget)None)
	    SendFocusNotify( descendant, FocusOut );
    }
    else {
	Boolean is_shell = XtIsSubclass(widget, shellWidgetClass);
	register XtEventRec* p = widget->core.event_table;
	register XtEventHandler proc = HandleFocus; /* compiler bug */
	EventMask eventMask =
	    FocusChangeMask |
		/* shells are always occluded by their children */
		((widget == XtParent(descendant) && is_shell)
			    ? 0 : EnterWindowMask | LeaveWindowMask);
	while (p != NULL && p->proc != proc) p = p->next;
	if (is_shell && p != NULL && p->mask != eventMask) {
	    XtRemoveEventHandler(widget, XtAllEvents, False,
				 HandleFocus, p->closure);
	    p = NULL;
	}
	if (p == NULL)
	    XtAddEventHandler
		(
		 widget,
		 eventMask,
		 False,
		 HandleFocus,
		 (caddr_t)descendant
		);
	else {
	    p->closure = (caddr_t)descendant;
	}
	if (XtIsRealized(descendant)) {	/* are his translations installed? */
	    AddForwardingHandler(widget, descendant);
	}
	else {			/* we'll have to wait 'till later */
	    XtAddEventHandler
		(
		 descendant,
		 StructureNotifyMask,
		 False,
		 QueryEventMask,
		 (caddr_t)widget
		);
	}
    }
    FocusTraceGood = False;
}


static SendFocusNotify(child, type)
    Widget child;
    int type;
{
    XEvent event;
    EventMask mask;
    GrabType grabType;

    if (_XtBuildEventMask(child) & FocusChangeMask) {
	event.xfocus.serial = LastKnownRequestProcessed(XtDisplay(child));
	event.xfocus.send_event = True;
	event.xfocus.display = XtDisplay(child);
	event.xfocus.window = XtWindow(child);
	event.xfocus.type = type;
	event.xfocus.mode = NotifyNormal;
	event.xfocus.detail = NotifyAncestor;
	ConvertTypeToMask(type, &mask, &grabType);
	DispatchEvent(&event, child, mask);
    }
}
