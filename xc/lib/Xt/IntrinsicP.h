/*
* $XConsortium: IntrinsicP.h,v 1.39 89/09/13 14:27:31 swick Exp $
* $oHeader: IntrinsicP.h,v 1.4 88/08/26 14:49:52 asente Exp $
*/

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

#ifndef _XtintrinsicP_h
#define _XtintrinsicP_h

#include "Intrinsic.h"

typedef unsigned long XtVersionType;

#define XT_VERSION 11
#define XT_REVISION 3
#define XtVersion (XT_VERSION * 1000 + XT_REVISION)
#define XtVersionDontCheck 0

extern void _XtInherit();
    /* takes no arguments */

typedef void (*XtProc)();
    /* takes no arguments */

typedef void (*XtWidgetClassProc)();
    /* WidgetClass class */

typedef void (*XtWidgetProc)();
    /* Widget widget */

typedef Boolean (*XtAcceptFocusProc)();
    /* Widget widget; */
    /* Time *time; */ /* X time */

typedef void (*XtArgsProc)();
    /* Widget   widget */
    /* ArgList  args */
    /* Cardinal *num_args */

typedef void (*XtInitProc)();
    /* Widget request_widget; */
    /* Widget new_widget; */

typedef Boolean (*XtSetValuesFunc)();  /* returns TRUE if redisplay needed */
    /* Widget widget;       */
    /* Widget request;      */
    /* Widget new;	    */

typedef Boolean (*XtArgsFunc)();
    /* Widget   widget      */
    /* ArgList  args	    */
    /* Cardinal *num_args   */

typedef void (*XtAlmostProc)();
    /* Widget		widget;     */
    /* Widget		new_widget; */
    /* XtWidgetGeometry *request;   */
    /* XtWidgetGeometry *reply;     */

typedef void (*XtExposeProc)();
    /* Widget    widget; */
    /* XEvent    *event; */
    /* Region	 region; */

/* compress_exposure options*/
#define XtExposeNoCompress		((XtEnum)False)
#define XtExposeCompressSeries		((XtEnum)True)
#define XtExposeCompressMultiple	2
#define XtExposeCompressMaximal		3

/* modifiers */
#define XtExposeGraphicsExpose	  	0x10
#define XtExposeGraphicsExposeMerged	0x20
#define XtExposeNoExpose	  	0x40


typedef void (*XtRealizeProc) ();
    /* Widget	widget;			    */
    /* XtValueMask mask;		    */
    /* XSetWindowAttributes *attributes;    */

typedef void (*XtCreatePopupChildProc)();

typedef XtGeometryResult (*XtGeometryHandler)();
    /* Widget		widget      */
    /* XtWidgetGeometry *request    */
    /* XtWidgetGeometry *reply      */

typedef void (*XtStringProc)();
    /* Widget		widget	    */
    /* String		str	    */

typedef struct _StateRec *StatePtr;

typedef struct _XtTMRec {
    XtTranslations  translations;	/* private to Translation Manager    */
    XtBoundActions  proc_table;		/* procedure bindings for actions    */
    StatePtr        current_state;      /* Translation Manager state ptr     */
    unsigned long   lastEventTime;
} XtTMRec, *XtTM;

#include "CoreP.h"
#include "CompositeP.h"
#include "ConstrainP.h"

#define XtDisplay(widget)	DisplayOfScreen((widget)->core.screen)
#define XtScreen(widget)	((widget)->core.screen)
#define XtWindow(widget)	((widget)->core.window)

#define XtDisplayOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.screen->display)
#define XtScreenOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.screen)
#define XtWindowOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.window)

#define XtClass(widget)		((widget)->core.widget_class)
#define XtSuperclass(widget)	(XtClass(widget)->core_class.superclass)
#define XtIsManaged(object)	(XtIsRectObj(object) ? (object)->core.managed : False)
#define XtIsRealized(object)	(XtWindowOfObject(object) != NULL)
#define XtIsSensitive(object) \
    (XtIsRect(object) ? ((object)->core.sensitive && \
			 (object)->core.ancestor_sensitive) : False)
#define XtParent(widget)	((widget)->core.parent)
#define XtName(object)		(XtIsWidget(object) ? (object)->core.name : XrmQuarkToString((object)->core.xrm_name))

#undef XtIsRectObj
#define XtIsRectObj(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x02)

#undef XtIsWidget
#define XtIsWidget(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x04)

#undef XtIsComposite
#define XtIsComposite(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x08)

#undef XtIsConstraint
#define XtIsConstraint(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x10)

#undef XtIsShell
#define XtIsShell(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x20)

#undef XtIsWMShell
#define XtIsWMShell(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x40)

#undef XtIsTopLevelShell
#define XtIsTopLevelShell(obj) \
    (((Object)obj)->object.widget_class->core_class.class_inited & 0x80)

#ifdef DEBUG
#define XtCheckSubclass(w, widget_class_ptr, message)	\
	if (!XtIsSubclass((w), (widget_class_ptr))) {	\
	    String params[3];				\
	    Cardinal num_params = 3;			\
	    params[0] = (w)->core.widget_class->core_class.class_name;	     \
	    params[1] = (widget_class_ptr)->core_class.class_name;	     \
	    params[2] = (message);					     \
	    XtErrorMsg("subclassMismatch", "xtCheckSubclass",		     \
		    "XtToolkitError",					     \
		    "Widget class %s found when subclass of %s expected: %s",\
		    params, &num_params);		\
	}
#else
#define XtCheckSubclass(w, widget_class, message)	/* nothing */
#endif

extern void XtCreateWindow ();
    /* Widget widget; */
    /* unsigned int windowClass; */
    /* Visual *visual; */
    /* Mask valueMask; */
    /* XSetWindowAttributes *attributes; */

extern void XtResizeWidget(); /* widget, width, height, borderWidth */
    /* Widget  widget */
    /* Dimension width, height, borderWidth; */

extern void XtMoveWidget(); /* widget, x, y */
    /* Widget  widget */
    /* Position x, y  */

extern void XtConfigureWidget(); /* widget, x, y, width, height, borderWidth */
    /* Widget widget; */
    /* Position x, y; */
    /* Dimension height, width, borderWidth; */

#endif /* _XtIntrinsicP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
