/*
 * $Header: Viewport.h,v 1.1 88/01/12 14:50:04 swick Exp $
 * Public definitions for Viewport widget
 */

#ifndef _Viewport_h
#define _Viewport_h

#include <X11/Form.h>

/* fields added to Form */
#define	XtNforceBars		"forceBars"
#define	XtNallowHoriz		"allowHoriz"
#define	XtNallowVert		"allowVert"
#define	XtNuseBottom		"useBottom"
#define	XtNuseRight		"useRight"

extern WidgetClass viewportWidgetClass;

typedef struct _ViewportClassRec *ViewportWidgetClass;
typedef struct _ViewportRec	 *ViewportWidget;

#endif _Viewport_h
