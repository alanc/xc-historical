#ifndef lint
static char rcsid[] = "$Header: Event.c,v 1.42 88/01/20 15:35:25 swick Locked $";
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

#include <X/Intrinsic.h>
#include <X/Xutil.h>
#include <X/Atoms.h>

EventMask _XtBuildEventMask(widget)
    Widget widget;
{
    _XtEventTable ev;
    EventMask	mask = 0;

    for (ev = widget->core.event_table; ev != NULL; ev = ev->next)
	mask |= ev->mask;
    if (widget->core.widget_class->core_class.expose != NULL)
	mask |= ExposureMask;
    if (widget->core.widget_class->core_class.visible_interest) 
	mask |= VisibilityChangeMask;

    return mask;
}

void XtRemoveEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    Opaque	closure;
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

void XtAddEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    Opaque	closure;
{
   register XtEventRec *p,**pp;
   EventMask oldMask;

   if (eventMask == 0 && other == FALSE) return;

   if (XtIsRealized(widget)) oldMask = _XtBuildEventMask(widget);

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

	p->next = widget->core.event_table;
	widget->core.event_table = p;

    } else {
	/* update existing proc */
	p->mask |= eventMask;
	p->non_filter = p->non_filter || other;
    }

    if (XtIsRealized(widget)) {
	EventMask mask = _XtBuildEventMask(widget);

	if (oldMask != mask)
	    XSelectInput(XtDisplay(widget), XtWindow(widget), mask);
    }

}

typedef struct _HashRec *HashPtr;

typedef struct _HashRec {
    Window	window;
    Widget	widget;
    HashPtr	next;
} HashRec;

int sizes[] = {1009, 2003, 4007, 8017, 16033, 32063, 64151, 128257};
#define NUMSIZES 8

typedef struct {
    unsigned int	sizeIndex;
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
    HashPtr hp, *hpp;

    if ((table->count + (table->count / 5)) >= table->size) ExpandTable();

    hpp = &table->entries[(unsigned int)window % table->size];
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

    hpp = &table->entries[(unsigned int)window % table->size];
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

    if (oldTable->sizeIndex == NUMSIZES) return;

    table = (HashTable) XtMalloc(
	(unsigned) sizeof(HashTableRec)
	+sizes[oldTable->sizeIndex+1]*sizeof(HashRec));
    table->sizeIndex = oldTable->sizeIndex+1;
    table->size = sizes[table->sizeIndex];
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


Widget XtWindowToWidget(window)
    Window window;
{
    HashPtr hp;

    for (
        hp = table->entries[(unsigned int)window % table->size];
        hp != NULL;
	hp = hp->next)
	if (hp->window == window) return hp->widget;

    return NULL;
}

static void InitializeHash()
{
    int i;

    table = (HashTable) XtMalloc(
        (unsigned) sizeof(HashTableRec)+sizes[0]*sizeof(HashPtr));

    table->sizeIndex = 0;
    table->size = sizes[0];
    table->count = 0;
    for (i=0; i<table->size; i++) table->entries[i] = NULL;
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


Region XRectToRegion(rect)
    register XRectangle *rect;
{
    Region region;
    register Box *box;

    region = ( Region )Xmalloc( (unsigned) sizeof( REGION ));
    box = region->rects = ( BOX * )Xmalloc( (unsigned) sizeof( BOX ));
    region->numRects = 1;
    box->x1 = region->extents.x1 = rect->x;
    box->y1 = region->extents.y1 = rect->y;
    box->x2 = region->extents.x2 = rect->x + rect->width;
    box->y2 = region->extents.y2 = rect->y + rect->height;
    region->size = 1;
    return( region );
}


XtAddExposureToRegion(event, region)
    XEvent   *event;
    Region   region;
{
    XRectangle rect;
    Region exposeRegion;

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

    exposeRegion = XRectToRegion(&rect);
    XUnionRegion(exposeRegion, region, region);
    XDestroyRegion(exposeRegion);
}


static void DispatchEvent(event, widget, mask)
    XEvent    *event;
    Widget    widget;
    unsigned long mask;


{
    XtEventRec *p;   
    XtEventHandler proc[100];
    Opaque closure[100];
    int numprocs, i;
    static Region exposeRegion = NULL;

    if (mask == ExposureMask) {
      if (widget->core.widget_class->core_class.compress_exposure) {
	if ((event->type == Expose && event->xexpose.count != 0) ||
	    (event->type == GraphicsExpose && event->xgraphicsexpose.count != 0)) {
	  /* %%% this code will need to be fixed to support multiple displays*/
	  if (exposeRegion == NULL) exposeRegion = XCreateRegion();
	  XtAddExposureToRegion(event, exposeRegion);
	  return;
	}
	if (event->type != NoExpose) {
	  XRectangle rect;
	  XClipBox(exposeRegion, &rect);
	  switch (event->type) {
	    case Expose:
		      event->xexpose.x = rect.x;
		      event->xexpose.y = rect.y;
		      event->xexpose.width = rect.width;
		      event->xexpose.height = rect.height;
		      break;
	    case GraphicsExpose:
		      event->xgraphicsexpose.x = rect.x;
		      event->xgraphicsexpose.y = rect.y;
		      event->xgraphicsexpose.width = rect.width;
		      event->xgraphicsexpose.height = rect.height;
		      break;
	  }
	  XDestroyRegion(exposeRegion);
	  exposeRegion = NULL;
	}
      }
      if(widget->core.widget_class->core_class.expose != NULL)
         (*(widget->core.widget_class->core_class.expose))(widget,event);
    }
    if ((mask == VisibilityNotify) &&
            !(widget->core.widget_class->core_class.visible_interest)) return;

    /* Have to copy the procs into an array, because calling one of them */
    /* might call XtRemoveEventHandler, which would break our linked list.*/
    numprocs = 0;
    for (p=widget->core.event_table; p != NULL; p = p->next)
	if ((mask & p->mask) != 0 || (mask == 0 && p->non_filter)) {
	    proc[numprocs] = p->proc;
	    closure[numprocs++] = p->closure;
	}

    for (i=0 ; i<numprocs ; i++)
	 /* use VMS bindings for parameters ???*/
	(*(proc[i]))(widget, closure[i], event);
}


static void ConvertTypeToMask (eventType,mask,grabType,sensitive)
    int eventType;
    EventMask *mask;
    GrabType *grabType;
    Boolean *sensitive;
    
{

static MaskRec masks[] = {
    {0,			pass,   not_sensitive}, /* shouldn't see 0      */
    {0,			pass,   not_sensitive}, /* shouldn't see 1      */
    {KeyPressMask,      remap,  is_sensitive},  /* KeyPress		*/
    {KeyReleaseMask,    remap,  is_sensitive},  /* KeyRelease		*/
    {ButtonPressMask,   remap,  is_sensitive},  /* ButtonPress		*/
    {ButtonReleaseMask, remap,  is_sensitive},  /* ButtonRelease	*/
    {PointerMotionMask | 
     Button1MotionMask | 
     Button2MotionMask |
     Button3MotionMask | 
     Button4MotionMask | 
     Button5MotionMask | 
     ButtonMotionMask,  ignore, is_sensitive},  /* MotionNotify		*/
    {EnterWindowMask,   ignore, is_sensitive},  /* EnterNotify		*/
    {LeaveWindowMask,   ignore, is_sensitive},  /* LeaveNotify		*/
    {FocusChangeMask,   ignore, is_sensitive},  /* FocusIn		*/
    {FocusChangeMask,   ignore, is_sensitive},  /* FocusOut		*/
    {KeymapStateMask,   ignore, not_sensitive}, /* KeymapNotify		*/
    {ExposureMask,      pass,   not_sensitive}, /* Expose		*/
    {ExposureMask,	pass,   not_sensitive}, /* GraphicsExpose       */
    {ExposureMask,	pass,   not_sensitive}, /* NoExpose		*/
    {VisibilityChangeMask,pass, not_sensitive}, /* VisibilityNotify     */
    {0,			pass,   not_sensitive}, /* shouldn't see CreateNotify */
    {StructureNotifyMask,pass,  not_sensitive}, /* DestroyNotify	*/
    {StructureNotifyMask,pass,  not_sensitive}, /* UnmapNotify		*/
    {StructureNotifyMask,pass,  not_sensitive}, /* MapNotify		*/
    {0,			pass,   not_sensitive},	/* MapRequest		*/
    {StructureNotifyMask,pass,  not_sensitive}, /* ReparentNotify       */
    {StructureNotifyMask,pass,  not_sensitive}, /* ConfigureNotify      */
    {0,			pass,   not_sensitive},	/* ConfigureRequest     */
    {StructureNotifyMask,pass,  not_sensitive}, /* GravityNotify	*/
    {0,			pass,   not_sensitive},	/* ResizeRequest	*/
    {StructureNotifyMask,pass,  not_sensitive}, /* CirculateNotify      */
    {0,			pass,   not_sensitive},	/* CirculateRequest     */
    {PropertyChangeMask,ignore, not_sensitive}, /* PropertyNotify       */
    {0,			ignore, not_sensitive},	/* SelectionClear       */
    {0,			ignore, not_sensitive},	/* SelectionRequest     */
    {StructureNotifyMask,pass,  not_sensitive}, /* SelectionNotify      */
    {ColormapChangeMask,ignore, not_sensitive}, /* ColormapNotify       */
    {0,			ignore, not_sensitive},	/* ClientMessage	*/
    {StructureNotifyMask,ignore,not_sensitive}, /* MappingNotify	*/
  };

    eventType &= 0x7f;	/* Events sent with XSendEvent have high bit set. */
    (*mask)      = masks[eventType].mask;
    (*grabType)  = masks[eventType].grabType;
    (*sensitive) = masks[eventType].sensitive;
    return;
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
    register Widget widget;
    EventMask   mask;
    GrabType    grabType;
    Boolean     sensitivity;
    register GrabList gl;

#define IsSensitive ((!sensitivity) || (widget->core.sensitive && widget->core.ancestor_sensitive))

    widget = XtWindowToWidget (event->xany.window);
    if (widget == NULL) return;

    ConvertTypeToMask(event->xany.type, &mask, &grabType, &sensitivity);

    if (grabType == pass) {
	DispatchEvent(event, widget, mask);

    } else if (grabList == NULL) {
	if IsSensitive {
	    DispatchEvent(event, widget, mask);
	}

    } else if (grabType == remap) {
        if (OnGrabList(widget) && IsSensitive) {
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
	if IsSensitive {
	    DispatchEvent(event, widget, mask);
	}
    }
}

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

    XtAddCallback(widget, XtNdestroyCallback, GrabDestroyCallback, NULL);
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

    XtRemoveCallback(widget, XtNdestroyCallback, GrabDestroyCallback, NULL);
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
