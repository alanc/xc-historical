#ifndef lint
static char rcsid[] = "$Header: Event.c,v 1.48 88/02/02 17:54:58 swick Locked $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include "IntrinsicI.h"
#include "Event.h"
#include "Atoms.h"
extern void bzero();

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
    Opaque	closure;
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
    Opaque	closure;
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
    Opaque	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, FALSE);
}


void XtAddEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    Opaque	closure;
{
    AddEventHandler(widget, eventMask, other, proc, closure, FALSE);
}


void XtRemoveRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    Opaque	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, TRUE);
}


void XtAddRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    Opaque	closure;
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


/* %%% the following typedefs are lifted from X/lib/X/region.h;
 * that file is too big to be included here.  XRectToRegion should
 * become a standard part of Xlib anyway...
 */

typedef struct {
    short x1, x2, y1, y2;
} BOX, Box;

typedef struct _XRegion {
    short size;
    short numRects;
    BOX *rects;
    BOX extents;
} REGION;


XUnionRectWithRegion(rect, source, dest)
    register XRectangle *rect;
    Region source, dest;
{
    REGION region;

    region.rects = &region.extents;
    region.numRects = 1;
    region.extents.x1 = rect->x;
    region.extents.y1 = rect->y;
    region.extents.x2 = rect->x + rect->width;
    region.extents.y2 = rect->y + rect->height;
    region.size = 1;

    XUnionRegion(&region, source, dest);
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


static void DispatchEvent(event, widget, mask)
    register XEvent    *event;
    Widget    widget;
    unsigned long mask;
{
    XtEventRec *p;   
    XtEventHandler proc[100];
    Opaque closure[100];
    int numprocs, i;
    XEvent nextEvent;
    static Region exposeRegion = NULL;

    if (mask == ExposureMask) {
	if (widget->core.widget_class->core_class.compress_exposure) {
	    if (event->xexpose.count != 0) {
		if (exposeRegion == NULL) exposeRegion = XCreateRegion();
		XtAddExposureToRegion(event, exposeRegion);
		return;
	    }
	    if (exposeRegion != NULL) {
		/* Patch event to have the new bounding box */
		XRectangle rect;
		XtAddExposureToRegion(event, exposeRegion);
		XClipBox(exposeRegion, &rect);
		event->xexpose.x = rect.x;
		event->xexpose.y = rect.y;
		event->xexpose.width = rect.width;
		event->xexpose.height = rect.height;
		XDestroyRegion(exposeRegion);
		exposeRegion = NULL;
	    }
	}
	if (widget->core.widget_class->core_class.expose != NULL) {
	    (*widget->core.widget_class->core_class.expose)(widget,event);
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
		    event->xcrossing.window == nextEvent.xcrossing.window &&
		    event->xcrossing.subwindow == nextEvent.xcrossing.subwindow) {
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
	    if (gl->widget == widget) return (TRUE);
	    if (gl->exclusive) break;
	}
    return (FALSE);
}


void XtDispatchEvent (event)
    XEvent  *event;
{
    register    Widget widget;
    EventMask   mask;
    GrabType    grabType;
    register    GrabList gl;

#define ShouldDispatch \
    (   (grabType == pass) \
     || (widget->core.sensitive && widget->core.ancestor_sensitive))

    widget = XtWindowToWidget (event->xany.display, event->xany.window);
    if (widget == NULL) return;

    /* Lint complains about &grabType not matching the declaration.
       Don't bother trying to fix it, it won't work */

    ConvertTypeToMask(event->xany.type, &mask, &grabType);

    if (grabType == pass) {
	DispatchEvent(event, widget, mask);

    } else if (grabList == NULL) {
	if ShouldDispatch {
	    DispatchEvent(event, widget, mask);
	}

    } else if (grabType == remap) {
        if (OnGrabList(widget) && ShouldDispatch) {
	    DispatchEvent(event, widget, mask);
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

    }  else if (OnGrabList(widget)) {
	if ShouldDispatch {
	    DispatchEvent(event, widget, mask);
	}
    }
}

/* ARGSUSED */
static void GrabDestroyCallback(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    /* Remove widget from grab list if it destroyed */
    XtRemoveGrab(widget);
}

void XtAddGrab(widget, exclusive, spring_loaded)
    Widget  widget;
    Boolean exclusive;
    Boolean spring_loaded;
{
    register GrabList    gl;

    if (spring_loaded && !exclusive) {
	XtWarning("XtAddGrab requires exclusive grab if spring_loaded is TRUE");
	exclusive = TRUE;
    }

    gl		      = XtNew(GrabRec);
    gl->next	      = grabList;
    grabList	      = gl;
    gl->widget        = widget;
    gl->exclusive     = exclusive;
    gl->spring_loaded = spring_loaded;

    XtAddCallback(
	widget, XtNdestroyCallback, GrabDestroyCallback, (Opaque)NULL);
}

void XtRemoveGrab(widget)
    Widget  widget;
{
    register GrabList gl, prev, next;

    for (prev = NULL, gl = grabList; gl != NULL; prev = gl, gl = gl->next) {
	if (gl->widget == widget) {
	    break;
	}
    }

    if (gl == NULL) {
      XtWarning("XtRemoveGrab asked to remove a widget not on the grab list");
      return;
    }

    while (gl) {
	if (gl->widget == widget) {
	    if (prev)
		prev->next = next = gl->next;
	    else
		grabList = next = gl->next;
	    XtFree(gl);
	    gl = next;
	}
	else {
	    prev = gl;
	    gl = gl->next;
	}
    }
    XtRemoveCallback(
	widget, XtNdestroyCallback, GrabDestroyCallback, (Opaque)NULL);
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
    DestroyList = NULL;
    InitializeHash();
}
