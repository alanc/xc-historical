e/*
 *	sccsid:	%W%	%G%
 */

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

#ifndef _Xtintrinsic_h
#define _Xtintrinsic_h

#include <X11/Xresource.h>
#include <X11/Xutil.h>

/****************************************************************
 ****************************************************************
 ***                                                          ***
 ***                                                          ***
 ***                   X Toolkit Intrinsics                   ***
 ***                                                          ***
 ***                                                          ***
 ****************************************************************
 ****************************************************************/


/****************************************************************
 *
 * System Dependent Definitions
 *
 *
 * The typedef for XtArgVal should be chosen such that
 *      sizeof (XtArgVal) == max (sizeof(caddr_t), sizeof(long))
 *
 * ArgLists rely heavily on the above typedef.
 *
 ****************************************************************/

typedef char *XtArgVal;

/***************************************************************
 * Widget Core Data Structure
 *
 *
 **************************************************************/

 typedef struct _Core {
        WidgetClass     widget_class;   /* pointer to Core Class data */
	Display		*dpy;		/* widget display connection */
	Screen		screen		/* widget screen */
	Window		window;		/* window ID */
	Widget		parent;		/* parent widget */
        String          name;
	XrmName		xrm_name;
        Boolean         managed;        /* is this widget controlled by geometry manager */
	Position 	x,y;		/* window position */
	Dimension	width,height;	/* window dimensions */
        Cardinal        depth;          /* number of planes in window */
        Boolean         visible;        /* is window mapped and not occluded */
	Pixel	        background_pixel; /* window background */
        Pixmap          background_pixmap; /* window background */
	Dimension     	border_width;	/* window border width */
	Pixel		border_pixel;	  /* window border pixel */
        Pixmap          border_pixmap;    /* window border pixmap */
	EventMask	event_mask;	/* events to select for */
        EventTable      event_table;    /* private to event dispatcher */
        Boolean         compress_motion; /* compress MotionNotify for this window */
	Boolean		sensitive;	/* is this widget sensitive */
        Boolean         ancestor_sensitive; /* are all ancestors sensitive */
        Translations    translations;    /* private to Translation Manager */
        CallbackList    destroy_callbacks; /* who to call if this widget destroyed */
        Boolean         being_destroyed;  /* marked for deletion */
} core;

/******************************************************************
 *
 * Core Class Structure. Widgets, regardless of their class, will have
 * these fields. All widgets of a given class will have the same values
 * for these fields. Widgets of a given class may also have additional
 * common fields. These additional fields are included in incremental
 * class structures, such as CommandClass.
 ********************************************************************/

 typedef struct _Class {
        WidgetClass    superclass;    /* pointer to superclass Class struct */
        String         class_name;    
        String         default_instance_name;
        Cardinal       size;          /* size for pickling */
        WidgetProc     initialize;    /* create a widget of this class */
        WidgetProc     realize;       /* realize a widget of this class */
	XtActionList   actions;       /* tokens to widget semantics bindings */
        XtResourceList resources;     /* resources for this class */
        Cardinal       num_resource;  /* number of resources in class list */
        XrmExtra       xrm_extra;     /* private for resource manager */
        XrmClass       xrm_class;     /* resource class */
        Boolean        visible_interest; 
	WidgetProc	destroy;	/* proc called to delete widget */
	WidgetProc	resize;		/* proc called to inform widget of position/dimension change */
	WidgetProc	expose		/* proc to call when window is exposed */
	WidgetArgProc	set_values;	/* proc called to set widget values */
	Boolean		accepts_focus;	/* does widget accept focus */
	WidgetProc	accept_focus;	/* proc called to give widget the focus */
	Boolean		composite;	/* is this a composite widget */
  } CoreClass, *WidgetClass;

/*********************************************************************
 *
 *  Additional fields for widgets of (super)Class 'Composite'
 *
 ********************************************************************/

 typedef struct _compositeClass { /* incremental additions to Core for composites */
	XtGeometryHandler	geometryMgr; 	/* geometry manager for children of widget */
	WidgetChildrenProc      add_children;   /* add widgets to managed status */
	WidgetChildrenProc      remove_children; /* remove widgets from managed status */
	WidgetProc		move_focus_to_next; /* move Focus to next child */
	WidgetProc		move_focus_to_prev; /* move Focus to previous child */
  } compositeClass;

/************************************************************************
 *
 * Additional instance variables for widgets of (super)class 'Composite' 
 *
 ************************************************************************

typedef struct _composite {
      WidgetChildren   children;  /* list of widget children (managed and unmanaged) */
      Cardinal         num_children; /* total number of widget children */
      Cardinal         num_managed_children; /* number of geometry managed children */
  } Composite;      
	

/*************************************************************************
 *
 * Generic Procedures
 *
 *************************************************************************/

extern WidgetClass coreClass; 
extern WidgetClass compositeClass; 

typedef void (*WidgetExposeProc)();
    /* Widget    widget; */
    /* XEvent    *event; */

extern Boolean XtIsSubclass ();
    /* Widget widget; */
    /* WidgetClass widgetClass; */

extern Widget XtWidgetCreate ();
    /* WidgetClass widgetClass; */
    /* Widget      parent; */
    /* ArgList     args; */
    /* Cardinal    argCount; */


extern void XtWidgetRealize ();
    /* Widget widget */


extern Boolean XtWidgetIsRealized ();
    /* Widget    widget; */

extern void XtWidgetDestroy ();
    /* Widget widget */

extern void XtWidgetSetSensitive ();
    /* Widget    widget; */
    /* Boolean   sensitive; */
/**********************************************************
 *
 * Composite widget Procedures
 *
 **********************************************************\

typedef void (*WidgetChildrenProc)();
    /* WidgetList children; */
    /* Cardinal childCount; */

extern void XtCompositeAddChildren ();
    /* WidgetList children; */
    /* Cardinal   childCount; */

extern void XtCompositeAddChild ();
    /* Widget    child; */

extern void XtCompositeRemoveChildren ();
    /* WidgetList children; */
    /* Cardinal   childCount; */

extern void XtCompositeRemoveChild ();
    /* Widget child; */


/*************************************************************
 *
 *  Callbacks
 *
 **************************************************************/

typdef void (*Callback)();
    /* Widget widget; */
    /* caddr_t closure; */

extern void XtAddCallback ();
    /* CallbackList callbacks; */
    /* Callback     callback; */
    /* Widget       widget;   */
    /* caddr_t      closure;  */


extern void XtRemoveCallback ();
    /* CallbackList callbacks; */
    /* Callback     callback; */
    /* Widget       widget;   */
    /* caddr_t      closure;  */


extern void XtRemoveAllXCallbacks ();
    /* CallbackList callbacks; */

extern void XtCallCallbacks ();
    /* Callbacks callbacks; */


/****************************************************************
 *
 * Miscellaneous definitions
 *
 ****************************************************************/

#include	<sys/types.h>

#ifndef NULL
#define NULL 0
#endif

#define Boolean int
#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define XtNumber(arr)			(sizeof(arr) / sizeof(arr[0]))

typedef char *String;

typedef unsigned long   GCMask;     /* Mask of values that are used by widget*/
typedef unsigned long   Pixel;	    /* Index into colormap	      */
typedef int		Position;   /* Offset from 0 coordinate	      */
typedef unsigned int	Dimension;  /* Size in pixels		      */
typedef Widget[]        WidgetList;
typedef void (*WidgetProc)(/* w */); /* Widget w */



/****************************************************************
 *
 * Toolkit initialization
 *
 ****************************************************************/

extern void    XtInitialize();
    /* XrmOptionsDescRec    options; */
    /* Cardinal             opt_count; */
    /* char               **argv;  */
    /* Cardinal             argc; */
    /* XrmName              name; */
    /* XrmClass             class; */
    /* XrmResourceDatabase  rdb; */
    /* Widget              *root; */

/****************************************************************
 *
 * Memory Management
 *
 ****************************************************************/

#ifndef _Alloc_c_

extern char *XtMalloc(); /* size */
    /* Cardinal size; */

extern char *XtCalloc(); /* num, size */
    /* Cardinal num, size; */

extern char *XtRealloc(); /* ptr, num */
    /* char     *ptr; */
    /* Cardinal num; */

extern void XtFree(); /* ptr */
	/* char  *ptr */


#endif




/****************************************************************
 *
 * Arg lists
 *
 ****************************************************************/

typedef struct {
    XrmAtom	name;
    XtArgVal	value;
} Arg, *ArgList;

#define XtSetArg(arg, n, d) \
    ( (arg).name = (n), (arg).value = (XtArgVal)(d) )

extern ArgList XtMergeArgLists(); /* args1, argCount1, args2, argCount2 */
    /* ArgList args1;       */
    /* int     argCount1;   */
    /* ArgList args2;       */
    /* int     argCount2;   */



/****************************************************************
 *
 * Event Management
 *
 ****************************************************************/


typedef void (*XtEventHandler)(); /* widget,event, closure */
    /* Widget  widget   */
    /* XEvent  *event;  */
    /* caddr_t closure  */

extern void XtSetEventHandler(); /* widget, proc, eventMask, closure , other */
    /* Widget		widget      */
    /* XtEventHandler   proc;       */
    /* EventMask        eventMask;  */
    /* caddr_t		closure ;   */
    /* Boolean          other;      */


extern void XtRemoveEventHandler(); /* widget, proc, closure */
    /* Widget    widget        */
    /* XtEventHandler proc;    */
    /* caddr_t   closure;      */


extern void XtDispatchEvent(); /* event */
    /* XEvent	*event; */

extern void XtMainLoop ();

/****************************************************************
 *
 * Event Gathering Routines
 *
 ****************************************************************/

extern XtIntervalId XtAddTimeout ();
    /* Widget widget; */
    /* unsigned long interval; */

extern void XtRemoveTimeOut ();
    /* XtIntervalId timer; */

extern unsigned long XtGetTimeOut ();
    /* XtIntervalId   timer; */

extern void XtAddInput(); /* widget, source, condition */
    /* Widget widget */
    /* int source; */
    /* caddr_t condition; */

extern void XtRemoveInput(); /* widget, source, condition */
    /* Widget widget */
    /* int source; */
    /* int condition; */

extern void XtNextEvent(); /* event */
    /* XtEvent *event; */

extern XtPeekEvent(); /* event */
    /* XtEvent *event; */

extern Boolean XtPending ();

extern XtSetCompressMotion ();
    /* Boolean compress; */

/****************************************************************
 *
 * Geometry Management
 *
 ****************************************************************/


#define    XtgeometryMove		/* Move window to wb.x, wb.y		*/
#define    XtgeometryResize		/* Resize window to wb.width, wb.height */
#define    XtgeometryStacking		/* Restack


typedef struct {
    GeometryMask request_mode;
    Position x, y;
    Dimension width, height, border_width;
    Widget sibling;
    int stack_mode;	/* Above, Below, TopIf, BottomIf, Opposite */
} WidgetGeometry;

typedef enum  {
    XtgeometryYes,        /* Request accepted. */
    XtgeometryNo,         /* Request denied. */
    XtgeometryAlmost,     /* Request denied, but willing to take replyBox. */
} XtGeometryReturnCode;


typedef void (*WidgetGeometryProc) ();
    /* Widget    widget; */
    /* WidgetGeometry geometry; */

typedef void (*XtGeometryHandler)();
    /*  widget, requestBox, replyBox */
    /* Widget		    widget */
    /* WidgetGeometry       *request */
    /* WidgetGeometry	    *reply   */


extern XtGeometryReturnCode XtMakeGeometryRequest();
    /*  widget, requestBox, replyBox */
    /* WidgetData	widget;			*/
    /* WidgetGeometry    *request;		*/
    /* WidgetGeometry	 *reply;   /* RETURN */

extern XtGeometryReturnCode XtMakeResizeRequest ();
    /* Widget    widget; */
    /* Dimension width, height; */
    /* WidgetGeometry *reply; */


/****************************************************************
 *
 * Graphic Context Management
 *****************************************************************/

extern GC XtGetGC(); /* widget, valueMask, values */
    /* Widget    widget */
    /* int       valueMask; */
    /* XGCValues *values; */

extern void XtDestroyGC ();
    /* GC gc; */


/****************************************************************
 *
 * Resources
 *
 ****************************************************************/

typedef struct _Resource {
    XrmAtom		resource_name;	/* Resource name		    */
    XrmAtom		resource_class;	/* Resource class		    */
    XrmAtom		resource_type;	/* Representation type desired      */
    Cardinal		resource_size;	/* Size in bytes of representation  */
    caddr_t		resource_offset;/* Where to put resource value      */
    XrmAtom		default_type;	/* representation type of specified default */
    caddr_t		default_addr;    /* Default resource value (or addr) */
} Resource, *ResourceList;


extern void XtGetResources();

    /* Widget       widget;             */
    /* ArgList	    args;		*/
    /* int	    argCount;		*/

extern void XtReadBinaryDatabase ();
    /* FILE    *f; */
    /* ResourceDatabase *db; */

extern void XtWidgetSetValues(); 
    /* Widget           widget;         */
    /* ArgList		args;		*/
    /* int		argCount;       */

extern void XtWidgetGetValues();
    /* Widget           widget;         */
    /* ArgList		args;		*/
    /* Cardinal 	argCount;       */

extern Widget XtNameToWidget ();
    /* String name ; */



/****************************************************************
 *
 * Translation Management
 *
 ****************************************************************/

typedef caddr_t XtEventsPtr;

typedef struct {
    char    *string;
    caddr_t value;
} XtActionsRec, *XtActionsPtr;

/* Different classes of action tokens */

typedef enum {XttokenChar, XttokenString,XttokenProc,\
       XttokenAtom,XttokenNumber} TokenType;

typdef struct _XtActionTokenRec {
    XtTokenType type;
    union {
       char c;
       char *str;
       XtAction action; 
    } value;
    struct _XtActionTokenRec *next;
} XtActionTokenRec, *XtActionTokenPtr;

extern caddr_t XtInterpretAtom ();
    /* TranslationPtr table; */
    /* XtAction action; */

/* List of tokens. */

typedef XrmQuark XtAction;
#define XtAtomToAction(atom)    ((XtAction) XrmAtomToQuark(atom))


extern XtEventsPtr XtSetActionBindings();
			/* eventTable, actionTable, defaultValue */
    /*  XtEventsPtr  eventTable;    */
    /*  XtActionsPtr actionTable;   */
    /*  caddr_t      defaultValue;  */

extern XtEventsPtr XtMergeEventBindings ();
    /* XtEventsPtr event1; */
    /* XtEventsPtr event2; */

extern XtEventsPtr XtParseEventBindings(); /* stringTable */
    /* char **stringTable */

extern XtActionTokenPtr XtTranslateEvent ();
    /* Widget    widget; */
    /* XKeyvent  *event; */
    /* XtTranslationTable table; */


/*************************************************************
 *
 * Error Handling
 *
 ************************************************************/


extern char* XtErrorDescrip (); /* errorNumber */
  /* int errorNumber */

extern void XtErrorHandler(); /* errorProc */
  /* int (*handler)(int); */

extern void XtErrorFunction (); /* errorNumber */
  /* int errorNumber */


#endif _Xtintrinsic_h
/* DON'T ADD STUFF AFTER THIS #endif */
