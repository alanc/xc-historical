/*
* $Header: IntrinsicP.h,v 6.16 88/01/29 13:00:12 asente Exp $
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
#ifndef _XtintrinsicP_h
#define _XtintrinsicP_h

#include "Intrinsic.h"

typedef unsigned long XtVersionType;

#define XT_VERSION 6
#define XT_REVISION 3
#define XtVersion (XT_VERSION * 1000 + XT_REVISION)

#define XtOffset(type,field)    ((unsigned int)&(((type)NULL)->field))

extern void _XtInherit();
    /* takes no arguments */

typedef void (*XtProc)();
    /* takes no arguments */

typedef void (*XtWidgetProc)();
    /* Widget widget */

typedef void (*XtWidgetClassProc)();
    /* WidgetClass class */

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

typedef void (*XtRealizeProc) ();
    /* Widget	widget;			    */
    /* XtValueMask mask;		    */
    /* XSetWindowAttributes *attributes;    */

typedef void (*XtCreatePopupChildProc)();

typedef XtGeometryResult (*XtGeometryHandler)();
    /* Widget		widget      */
    /* XtWidgetGeometry *request    */
    /* XtWidgetGeometry *reply      */

#include "CoreP.h"
#include "CompositeP.h"
#include "ConstraintP.h"

#define XtDisplay(widget)	((widget)->core.screen->display)
#define XtScreen(widget)	((widget)->core.screen)
#define XtWindow(widget)	((widget)->core.window)
#define XtClass(widget)		((widget)->core.widget_class)
#define XtSuperclass(widget)	(XtClass(widget)->core_class.superclass)
#define XtIsManaged(widget)     ((widget)->core.managed)
#define XtIsRealized(widget)	((widget)->core.window != NULL)
#define XtMapWidget(widget)	XMapWindow(XtDisplay(widget), XtWindow(widget))
#define XtUnmapWidget(widget)	\
		XUnmapWindow(XtDisplay(widget), XtWindow(widget))

extern void XtCreateWindow ();
    /* Widget widget; */
    /* unsigned int windowClass; */
    /* Visual *visual; */
    /* Mask valueMask; */
    /* XSetWindowAttributes *attributes; */

extern void XtResizeWidget(); /* widget, width, height, borderWidth */
    /* Widget  widget */
    /* Dimension width, height, borderWidth; */

extern void XtConfigureWidget(); /* widget, x, y, width, height, borderWidth */
    /* Widget widget; */
    /* Position x, y; */
    /* Dimension height, width, borderWidth; */

extern void XtMoveWidget(); /* widget, x, y */
    /* Widget  widget */
    /* Position x, y  */

/****************************************************************
 *
 * Graphic Context Management
 *****************************************************************/

extern GC XtGetGC(); /* widget, valueMask, values */
    /* Widget    widget */
    /* XtGCMask valueMask; */
    /* XGCValues *values; */

extern void XtDestroyGC (); /* widget, gc */
    /* Widget widget; */
    /* GC gc; */
/* we pass in the widget because XFreeGC needs a display, and there isn't */
/* one stored in the GC record. */

typedef struct _XtResource {
    String     resource_name;	/* Resource name			    */
    String     resource_class;	/* Resource class			    */
    String     resource_type;	/* Representation type desired		    */
    Cardinal    resource_size;	/* Size in bytes of representation	    */
    Cardinal    resource_offset;/* Offset from base to put resource value   */
    String     default_type;	/* representation type of specified default */
    caddr_t     default_addr;   /* Address of default resource		    */
} XtResource;

extern void XtReadBinaryDatabase ();
    /* FILE    *f;			*/
    /* ResourceDatabase *db;		*/

extern void XtWriteBinaryDatabase ();
    /* FILE    *f;			*/
    /* ResourceDatabase db;		*/

#endif _XtIntrinsicP_h
/* DON'T ADD STUFF AFTER THIS #endif */
