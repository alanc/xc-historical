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
	WType		wType;		/* widget type */
	Display		*dpy;		/* widget display connection */
	Screen		screen		/* widget screen */
	Window		window;		/* window ID */
	Widget		parent;		/* parent widget */
	XrmName		name;		/* full name */
	XrmClass	class;		/* full class */
	Position 	x,y;		/* window position */
	Dimension	width,height;	/* window dimensions */
	Dimension     	borderWidth;	/* window border width */
	Pixel	        background_pixel; /* window background */
	Pixel		border_pixel;	  /* window border pixel */
	Colormap	colormap;
	Boolean		sensitive;	/* is this widget accepting events */
	Boolean	      	mapped;		/* is window mapped */
	Boolean	      	visible;	/* is window visible */
	Cursor		cursor;		/* window cursor */
	Mask		event_mask;	/* events to select for */
	XtEventsPtr   	eventBindings;  /* events to tokens bindings */
	XtActionsPtr	actionBindings; /* tokens to widget semantics bindings */
	TranslationTable translationTable; /* compiled form of translation state */
	WidgetProc	expose		/* proc to call when window is exposed */
	WidgetProc	destroy;	/* proc called to delete widget */
	WidgetProc	resize;		/* proc called to inform widget of position/dimension change */
	WidgetProc	SetSensitivity; /* proc called to change widget sensitivity */
	WidgetArgProc	GetValues;	/* proc called to get widget values */
	WidgetArgProc	SetValues;	/* proc called to set widget values */
	Boolean		acceptsFocus;	/* does widget accept focus */
	WidgetProc	acceptFocus;	/* proc called to give widget the focus */
	WidgetProc	realize;	/* proc called to realize the widget */
	Boolean		composite;	/* is this a composite widget */
  } core;

 typedef struct _composite {
	XtGeometryHandler	geometryMgr; 	/* geometry manager for children of widget */
	unsigned int		numChildren;	/* how many children */
	GetChildrenProc		getChildren;	/* proc to get children information */
	IndexToChildProc	indexToChild;
	ChildToIndexProc	childToIndex;
	WidgetChildProc		AddSubWidget;	/* add widget to comosite */
	WidgetChildProc		DelSubWidget;  	/* delete widget from this composite */
	WidgetProc		moveFocusToNext; /* move Focus to next child */
	WidgetProc		moveFocusToPrev; /* move Focus to previous child */
  } composite;

	
	

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


/****************************************************************
 *
 * Error codes
 *
 ****************************************************************/

typedef	int XtStatus;

#define XtSUCCESS 0	/* No error. */

extern int (*XtErrorFunction)();
extern int XtreferenceCount;


#define XtNOMEM	  1		/* Out of memory. */
#define XtFOPEN   3		/* fopen failed. */
#define XtNOTWINDOW 4		/* Given window id is invalid. */


/****************************************************************
 *
 * Toolkit initialization
 *
 ****************************************************************/

extern void    XtInitialize();

/****************************************************************
 *
 * Memory Allocation
 *
 ****************************************************************/

#ifndef _Alloc_c_

extern char *XtMalloc(); /* size */
    /* unsigned size; */

extern char *XtCalloc(); /* num, size */
    /* unsigned num, size; */

extern char *XtRealloc(); /* ptr, num */
    /* char     *ptr; */
    /* unsigned num; */

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

typedef enum {
    Xtcontinue,
    XtstopProcessing,
} XtEventReturnCode;

#define XtINPUT_READ	(1)
#define XtINPUT_WRITE	(2)
#define XtINPUT_EXCEPT	(4)

/* These are hand-generated atoms for use in ClientMessage events
 * They are never sent to the server. They will not conflict with
 * any server-generated atoms because one of the 3 most significant
 * bits is set. We should consider getting real atoms from the server
 * at runtime 
 */

#define XtHasInput (0x80000001)
/* XClientEvent.data.l[0] is source
 * XClientEvent.data.l[1] is condition
 */

#define XtTimerExpired (0x80000002)
/* XClientEvent.data.l[0] is cookie
 */

typedef XtEventReturnCode (*XtEventHandler)(); /* widget,event, closure */
    /* Widget  widget   */
    /* XEvent  *event;  */
    /* caddr_t closure  */

extern void XtRegisterEventHandler(); /* widget, proc, eventMask, closure */
    /* Widget		widget      */
    /* XtEventHandler   proc;       */
    /* unsigned long    eventType;  */
    /* caddr_t		closure ;   */

extern void XtRegisterGlobalEventHandler(); /* dpy, proc, eventMask, closure */
    /* Display		*dpy;	    */
    /* XtEventHandler   proc;       */
    /* unsigned long    eventType   */
    /* caddr_t		data;       */

extern void XtDeleteEventHandler(); /* widget, proc */
    /* Widget    widget        */
    /* XtEventHandler proc;    */

extern void XtClearAllEventHandlers(); /* widget */
    /* Widget  widget */


extern void XtDispatchEvent(); /* event */
    /* XEvent	*event; */

extern void XtMainLoop ();

/****************************************************************
 *
 * Event Gathering Routines
 *
 ****************************************************************/

extern XtTimerID XtTimerCreate(); /* widget, interval */
    /* Widget widget */
    /* long interval; */

extern long XtGetTimeOut(); /* timerID */
    /* XtTimerID timerID */

extern int XtTimerDelete(); /* timerID */
    /* XtTimerID timerID */

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


/****************************************************************
 *
 * Geometry Management
 *
 ****************************************************************/


#define    XtgeometryMove		/* Move window to wb.x, wb.y		*/
#define    XtgeometryResize		/* Resize window to wb.width, wb.height */
#define    XtgeometryStacking		/* Restack


typedef struct {
    Position x, y;
    Dimension width, height, borderWidth;
    Widget sibling;
    int stackmode;	/* Above, Below, TopIf, BottomIf, Opposite */
} WidgetGeometry;

typedef enum  {
    XtgeometryYes,        /* Request accepted. */
    XtgeometryNo,         /* Request denied. */
    XtgeometryAlmost,     /* Request denied, but willing to take replyBox. */
    XtgeometryNoManager   /* Request denied: couldn't find geometry manager */
} XtGeometryReturnCode;

typedef XtGeometryReturnCode (*XtGeometryHandler)();
    /*  widget, request, requestBox, replyBox */
    /* Widget		    widget */
    /* int	            requestType; (any combination of Xtgeometry{Move,Resize,Stacking}) */
    /* WidgetGeometry       *request */
    /* WidgetGeometry	    *reply   */


extern XtGeometryReturnCode XtMakeGeometryRequest();
    /*  widget, request, requestBox, replyBox */
    /* WidgetData	widget;			*/
    /* int              requestType;		*/
    /* WidgetGeometry    *request;		*/
    /* WidgetGeometry	 *reply;   /* RETURN */



/****************************************************************
 *
 * Graphic Context Management
 *****************************************************************/

extern GC XtGetGC(); /* widget, valueMask, values */
    /* Widget    widget */
    /* int       valueMask; */
    /* XGCValues *values; */


/****************************************************************
 *
 * Send Event
 *
 ****************************************************************/

extern XtEventReturnCode XtSendEvent(); /* widget, type */
    /* Widget widget */
    /* unsigned long type; */


/****************************************************************
 *
 * Resources
 *
 ****************************************************************/

typedef struct _Resource {
    XrmAtom		name;		/* Resource name		    */
    XrmAtom		class;		/* Resource class		    */
    XrmAtom		type;		/* Representation type desired      */
    unsigned int	size;		/* Size in bytes of representation  */
    caddr_t		addr;		/* Where to put resource value      */
    XrmAtom		defaultType;	/* representation type of specified default */
    caddr_t		defaultaddr;    /* Default resource value (or addr) */
} Resource, *ResourceList;


extern void XtGetResources();
    /* resources, resourceCount, args, argCount,
       parent, widgetName, widgetClass, names, classes */
    /* ResourceList resources;		*/
    /* int	    resourceCount;      */
    /* ArgList	    args;		*/
    /* int	    argCount;		*/
    /* Widget	    parent;		*/
    /* XrmAtom	    widgetName;		*/
    /* XrmAtom	    widgetClass;	*/
    /* XrmNameList   *names;   /* RETURN */
    /* XrmClassList  *classes; /* RETURN */

extern void XtSetValues(); /* resources, resourceCount, args, argCount */
    /* ResourceList	resources;      */
    /* int		resourceCount;  */
    /* ArgList		args;		*/
    /* int		argCount;       */

extern void XtGetValues(); /* resources, resourceCount, args, argCount */
    /* ResourceList	resources;      */
    /* int		resourceCount;  */
    /* ArgList		args;		*/
    /* int		argCount;       */

extern int XtDefaultFGPixel, XtDefaultBGPixel;




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

typedef enum {XttokenChar, XttokenString, XttokenAction,XttokenNumber} TokenType;

/* List of tokens. */

typedef XrmQuark XtAction;
#define XtAtomToAction(atom)    ((XtAction) XrmAtomToQuark(atom))

typedef struct _XtActionTokenRec {
    TokenType type;
    union {
	char     c;
	char     *str;
	XtAction action;
    } value;
    struct _XtActionTokenRec *next;
} XtActionTokenRec, *XtActionTokenPtr;

typedef struct _TranslationRec *TranslationTable;

extern XtEventsPtr XtSetActionBindings();
			/* eventTable, actionTable, defaultValue */
    /*  XtEventsPtr  eventTable;    */
    /*  XtActionsPtr actionTable;   */
    /*  caddr_t      defaultValue;  */

extern XtEventsPtr XtParseEventBindings(); /* stringTable */
    /* char **stringTable */

extern caddr_t XtInterpretAction(); /* state, action */
    /* TranslationTable start;    */
    /* XtAction       action;   */

extern XtActionTokenPtr XtTranslateEvent(); /*widget, event, table */
    /* Widget	      widget    */
    /* XEvent         *event;   */
    /* TranslationTable table */


extern void XtDefineTranslation(); /* widget, stringTable */
    /*Widegt widget */
    /* char **stringTable */


/*************************************************************
 *
 * Error Handling
 *
 ************************************************************/

extern char* XtErrorDescrip (); /* errorNumber */
  /* int errorNumber */

extern void XtErrorHandler(); /* errorProc */
  /* (int (*Proc(int)) */

extern void XtErrorFunction (); /* errorNumber */
  /* int errorNumber */


#endif _Xtintrinsic_h
/* DON'T ADD STUFF AFTER THIS #endif */
