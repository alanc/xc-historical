/*
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
 * Miscellaneous definitions
 *
 ****************************************************************/


#include "/usr/src/x11/include/Xlib.h"
#include "/usr/src/x11/include/Xresource.h"
#include	<sys/types.h>

#ifndef NULL
#define NULL 0
#endif


#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define XtNumber(arr)			(sizeof(arr) / sizeof(arr[0]))
#define Offset(type,field)    ((unsigned int)&(((type)NULL)->field))
typedef char *String;
typedef struct _EventRec *EventTable;
typedef struct _GrabRec  *GrabList;
typedef unsigned long   Cardinal;
typedef int             Boolean;
typedef char*           Opaque;
typedef struct _TranslationData	*Translations;
typedef struct _CallbackRec*    CallbackList;
typedef unsigned long   ValueMask;
typedef unsigned long   XtIntervalId;
typedef unsigned long   GeometryMask;
typedef unsigned long   GCMask;     /* Mask of values that are used by widget*/
typedef unsigned long   EventMask;
typedef unsigned long   Pixel;	    /* Index into colormap	      */
typedef int		Position;   /* Offset from 0 coordinate	      */
typedef int		Dimension;  /* Size in pixels		      */

typedef void (*WidgetProc)();
    /* Widget widget */

typedef void (*SetValuesProc)();
    /* Widget widget;     */
    /* Widget new_values; */

typedef void (*ExposeProc)();
    /* Widget    widget; */
    /* XEvent    *event; */

typedef void (*RealizeProc) ();
    /* Widget	widget;			    */
    /* ValueMask mask;			    */
    /* XSetWindowAttributes *attributes;    */

typedef enum  {
    XtgeometryYes,        /* Request accepted. */
    XtgeometryNo,         /* Request denied. */
    XtgeometryAlmost,     /* Request denied, but willing to take replyBox. */
} XtGeometryReturnCode;

typedef XtGeometryReturnCode (*XtGeometryHandler)();
    /* Widget		    widget */
    /* WidgetGeometry       *request */
    /* WidgetGeometry	    *reply   */

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
 * Widget Core Data Structures
 *
 *
 **************************************************************/

typedef struct _CorePart {
    struct _WidgetClassRec *widget_class;/* pointer to Widget's ClassRec     */
    Screen	 *screen;	       /* window's screen		     */
    Window	 window;	       /* window ID			     */
    struct _CompositeRec   *parent;    /* parent widget		  	     */
    String       name;		       /* widget resource name		     */
    XrmName      xrm_name;	       /* widget resource name quarkified    */
    Boolean      managed;	       /* is widget geometry managed?        */
    Position     x, y;		       /* window position		     */
    Dimension    width, height;	       /* window dimensions		     */
    Cardinal     depth;		       /* number of planes in window         */
    Boolean      visible;	       /* is window mapped and not occluded? */
    Pixel	 background_pixel;     /* window background pixel	     */
    Pixmap       background_pixmap;    /* window background pixmap or NULL   */
    Dimension    border_width;	       /* window border width		     */
    Pixel	 border_pixel;	       /* window border pixel		     */
    Pixmap       border_pixmap;	       /* window border pixmap or NULL       */
    EventMask    event_mask;	       /* events to select for		     */
    EventTable   event_table;	       /* private to event dispatcher        */
    Boolean	 sensitive;	       /* is widget sensitive to user events */
    Boolean      ancestor_sensitive;   /* are all ancestors sensitive?       */
    Translations translations;	       /* private to Translation Manager     */
    CallbackList destroy_callbacks;    /* who to call when widget destroyed  */
    Boolean      being_destroyed;      /* marked for destroy		     */
} CorePart;

typedef struct _WidgetRec {
    CorePart    core;
 } WidgetRec, *Widget;

typedef Widget *WidgetList;

/* Some macros to get frequently used components of a widget */

#define XtDisplay(w)    (w->core.screen->display)
#define XtScreen(w)     (w->core.screen)
#define XtWindow(w)     (w->core.window)
#define XtMapWidget(w)  XMapWindow(XtDisplay(w), XtWindow(w))
#define XtUnmapWidget(w) XUnmapWindow(XtDisplay(w), XtWindow(w))

/******************************************************************
 *
 * Core Class Structure. Widgets, regardless of their class, will have
 * these fields.  All widgets of a given class will have the same values
 * for these fields.  Widgets of a given class may also have additional
 * common fields.  These additional fields are included in incremental
 * class structures, such as CommandClass.
 *
 * The fields that are specific to this subclass, as opposed to fields that
 * are part of the superclass, are called "subclass fields" below.  Many
 * procedures are responsible only for the subclass fields, and not for
 * any superclass fields.
 *
 ********************************************************************/

typedef struct _CoreClassPart {
    struct _WidgetClassRec *superclass;/* pointer to superclass ClassRec     */
    String      class_name;	       /* widget resource class name         */
    Cardinal    size;		       /* size in bytes of widget record     */
    WidgetProc  class_initialize;      /* class initialization proc	     */
    Boolean     class_inited;	       /* has class been initialized?        */
    WidgetProc  initialize;	       /* initialize subclass fields         */
    RealizeProc realize;	       /* XCreateWindow for widget	     */
    struct _XtActionsRec *actions;     /* widget semantics name to proc map  */
    Cardinal    num_actions;	       /* number of entries in actions       */
    struct _Resource *resources;       /* resources for subclass fields      */
    Cardinal    num_resources;	       /* number of entries in resources     */
    XrmClass    xrm_class;	       /* resource class quarkified	     */
    Boolean     compress_motion;       /* compress MotionNotify for widget   */
    Boolean     compress_exposure;     /* compress Expose events for widget  */
    Boolean     visible_interest;      /* select for VisibilityNotify        */
    WidgetProc  destroy;	       /* free data for subclass pointers    */
    WidgetProc  resize;		       /* geom manager changed widget size   */
    ExposeProc  expose;		       /* rediplay window		     */
    SetValuesProc	set_values;    /* set subclass resource values       */
    WidgetProc	accept_focus;	       /* assign input focus to widget       */
  } CoreClassPart;

typedef struct _WidgetClassRec {
    CoreClassPart core_class;
} WidgetClassRec, *WidgetClass;

extern WidgetClassRec widgetClassRec;
extern WidgetClass widgetClass;

/************************************************************************
 *
 * Additional instance fields for widgets of (sub)class 'Composite' 
 *
 ************************************************************************/

typedef Cardinal (*OrderProc)();
    /* Widget child; */


typedef struct _CompositePart {
    WidgetList  children;	/* array of ALL widget children		     */
    Cardinal    num_children;	/* total number of widget children 	     */
    Cardinal    num_slots;      /* number of slots in children array	     */
    Cardinal    num_managed_children; /* number of geometry managed children */
    OrderProc   insert_position;/* compute position new child should go into */
} CompositePart;

typedef struct _CompositeRec {
    CorePart      core;
    CompositePart composite;
} CompositeRec, *CompositeWidget;

typedef struct _ConstraintPart {
    caddr_t     mumble;		/* No new fields, keep C compiler happy */
} ConstraintPart;

typedef struct _ConstraintRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart  constraint;
} ConstraintRec, *ConstraintWidget;

/*********************************************************************
 *
 *  Additional class fields for widgets of (sub)class 'Composite'
 *
 ********************************************************************/

typedef struct _CompositeClassPart {
    XtGeometryHandler	geometry_manager;  /* geometry manager for children  */
    WidgetProc  change_managed;	       /* changed managed status of children */
    WidgetProc  insert_child;	       /* physically add child to parent     */
    WidgetProc  delete_child;	       /* physically remove child	     */
    WidgetProc  move_focus_to_next;    /* move Focus to next child	     */
    WidgetProc  move_focus_to_prev;    /* move Focus to previous child       */
} CompositeClassPart;

typedef struct _CompositeClassRec {
     CoreClassPart      core_class;
     CompositeClassPart composite_class;
} CompositeClassRec, *CompositeWidgetClass;

extern CompositeClassRec compositeClassRec;
extern CompositeWidgetClass compositeWidgetClass;


typedef struct _ConstraintClassPart {
    struct _Resource *constraints;    /* constraint resource list	     */
    Cardinal   num_constraints;       /* number of constraints in list       */
} ConstraintClassPart;

typedef struct _ConstraintClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
} ConstraintClassRec, *ConstraintWidgetClass;

extern ConstraintClassRec constraintClassRec;
extern ConstraintWidgetClass constraintWidgetClass;

/*************************************************************************
 *
 * Generic Procedures
 *
 *************************************************************************/


extern Boolean XtIsSubclass ();
    /* Widget       widget;	    */
    /* WidgetClass  widgetClass;    */

#define XtSuperclass(widget) \
    ((widget)->core.widget_class->core_class.superclass)

extern Widget XtCreateWidget ();
    /* String	    name;	    */
    /* WidgetClass  widgetClass;    */
    /* Widget       parent;	    */
    /* ArgList      args;	    */
    /* Cardinal     argCount;       */

extern Widget TopLevelCreate (); /*hack for now*/
    /* String	   name; */
    /* WidgetClass widgetClass; */
    /* Screen      *screen;*/
    /* ArgList     args; */
    /* Cardinal    argCount; */



extern void XtRealizeWidget ();
    /* Widget    widget      */

extern Boolean XtIsRealized ();
    /* Widget    widget; */

extern void XtDestroyWidget ();
    /* Widget widget */

extern void XtSetSensitive ();
    /* Widget    widget;    */
    /* Boolean   sensitive; */

/**********************************************************
 *
 * Composite widget Procedures
 *
 **********************************************************\


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

typedef void (*CallbackProc)();
    /* Widget widget; */
    /* caddr_t closure; */

typedef struct _CallbackRec {
    struct _CallbackRec *next;
    Widget   widget;
    CallbackProc callback;
    Opaque  closure;
}CallbackRec;


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


extern void XtRemoveAllCallbacks ();
    /* CallbackList callbacks; */

extern void XtCallCallbacks ();
    /* Callbacks callbacks; */


/****************************************************************
 *
 * Toolkit initialization
 *
 ****************************************************************/

extern Display *XtInitialize();
    /* XrmOptionsDescRec    options;    */
    /* Cardinal             opt_count;  */
    /* Cardinal		    *argc; */ /* returns count of args not processed */
    /* char		    **argv;     */
    /* XtAtom		    name;       */
    /* XtAtom		    class;      */
    /* Widget		    *root; */ /*returns top-level application widget */

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

/* ||| Much of this should be private */
CallbackList DestroyList;
Display *toplevelDisplay;

typedef enum {pass,ignore,remap} GrabType;
typedef void (*XtEventHandler)(); /* widget,event, closure */
    /* Widget  widget   */
    /* XEvent  *event;  */
    /* caddr_t closure  */

typedef struct _EventRec {
     struct _EventRec  *next;
     EventMask   mask;
     Boolean     non_filter;
     XtEventHandler proc;
     Opaque closure;
}EventRec;

typedef struct _GrabRec {
    struct _GrabRec *next;
    Widget  widget;
    Boolean  exclusive;
}GrabRec;

typedef struct _MaskRec {
    EventMask   mask;
    GrabType    grabType;
    Boolean     sensitive;
}MaskRec;
#define is_sensitive TRUE
#define not_sensitive FALSE
GrabRec *grabList;

extern void XtSetEventHandler(); /* widget, eventMask, other, proc, closure */
    /* Widget		widget      */
    /* EventMask        eventMask;  */
    /* Boolean          other;      */
    /* XtEventHandler   proc;       */
    /* caddr_t		closure ;   */


extern void XtRemoveEventHandler(); /* widget, proc, closure */
    /* Widget    widget        */
    /* XtEventHandler proc;    */
    /* caddr_t   closure;      */


extern void XtDispatchEvent(); /* event */
    /* XEvent	*event; */

extern void XtMainLoop();

/****************************************************************
 *
 * Event Gathering Routines
 *
 ****************************************************************/

#define XtINPUT_READ	(1)
#define XtINPUT_WRITE	(2)
#define XtINPUT_EXCEPT	(3)

extern Atom XtHasInput;
extern Atom XtTimerExpired;

extern XtIntervalId XtAddTimeOut();
    /* Widget widget;		*/
    /* unsigned long interval;  */

extern void XtRemoveTimeOut();
    /* XtIntervalId timer;      */

extern unsigned long XtGetTimeOut();
    /* XtIntervalId   timer;    */

extern void XtAddInput(); /* widget, source, condition */
    /* Widget widget		*/
    /* int source;		*/
    /* caddr_t condition;       */

extern void XtRemoveInput(); /* widget, source, condition */
    /* Widget widget		*/
    /* int source;	    */
    /* int condition;		*/

extern void XtNextEvent(); /* event */
    /* XtEvent *event;		*/

extern XtPeekEvent(); /* event */
    /* XtEvent *event;		*/

extern Boolean XtPending ();


/****************************************************************
 *
 * Geometry Management
 *
 ****************************************************************/

typedef struct {
    GeometryMask request_mode;
    Position x, y;
    Dimension width, height, border_width;
    Widget sibling;
    int stack_mode;	/* Above, Below, TopIf, BottomIf, Opposite */
} WidgetGeometry;


extern XtGeometryReturnCode XtMakeGeometryRequest();
    /*  widget, request, reply		    */
    /* Widget	widget; 		    */
    /* WidgetGeometry    *request;	    */
    /* WidgetGeometry	 *reply;  /* RETURN */

extern XtGeometryReturnCode XtMakeResizeRequest ();
    /* Widget    widget;	*/
    /* Dimension width, height; */
    /* WidgetGeometry *reply;   */

extern void XtResizeWidget(); /* widget */
    /* Widget  widget */

extern void XtMoveWidget(); /* widget, x, y */
    /* Widget  widget */
    /* Position x, y  */


/****************************************************************
 *
 * Graphic Context Management
 *****************************************************************/

extern GC XtGetGC(); /* widget, valueMask, values */
    /* Widget    widget */
    /* int       valueMask; */
    /* XGCValues *values; */

extern void XtDestroyGC ();
    /* Widget    widget */
    /* GC gc; */


/****************************************************************
 *
 * Resources
 *
 ****************************************************************/

typedef struct _Resource {
    XrmAtom     resource_name;	/* Resource name			    */
    XrmAtom     resource_class;	/* Resource class			    */
    XrmAtom     resource_type;	/* Representation type desired		    */
    Cardinal    resource_size;	/* Size in bytes of representation	    */
    Cardinal    resource_offset;/* Offset from base to put resource value   */
    XrmAtom     default_type;	/* representation type of specified default */
    caddr_t     default_addr;   /* Address of default resource		    */
} Resource, *ResourceList;


extern void XtGetResources();
    /* Widget       widget;             */
    /* ArgList	    args;		*/
    /* int	    argCount;		*/

extern void XtReadBinaryDatabase ();
    /* FILE    *f;			*/
    /* ResourceDatabase *db;		*/

extern void XtSetValues(); 
    /* Widget           widget;         */
    /* ArgList		args;		*/
    /* int		argCount;       */

extern void XtGetValues();
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

typedef struct _XtActionsRec{
    char    *string;
    caddr_t value;
} XtActionsRec, *XtActionsPtr;

/* ||| Should be private */
extern void XtDefineTranslation ();
    /* Widget widget */

/*************************************************************
 *
 * Error Handling
 *
 ************************************************************/


extern void XtSetErrorHandler(); /* errorProc */
  /* (*errorProc)(String); */

extern void XtError();  /* message */
    /* String message */

extern void XtSetWarningHandler(); /* errorProc */
  /* (*errorProc)(String); */

extern void XtWarning();  /* message */
    /* String message */


#endif _Xtintrinsic_h
/* DON'T ADD STUFF AFTER THIS #endif */
